// Asegúrate de que esta ruta coincida con el nombre de la carpeta 'include' de tu paquete
#include "controller_pkg/controller_node.hpp" 

ControllerNode::ControllerNode() : Node("controller_node")
{
  // 1. Suscriptor de Odometría (Desde Zenoh)
  odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
    "odom", 10, [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
      std::lock_guard<std::mutex> lock(data_mutex_);
      current_x_ = msg->pose.pose.position.x;
      current_y_ = msg->pose.pose.position.y;

      // Extraer Yaw (Rotación Z) del Cuaternión
      tf2::Quaternion q(
        msg->pose.pose.orientation.x,
        msg->pose.pose.orientation.y,
        msg->pose.pose.orientation.z,
        msg->pose.pose.orientation.w);
      tf2::Matrix3x3 m(q);
      double roll, pitch;
      m.getRPY(roll, pitch, current_yaw_);
      
      has_odom_ = true;
    });

  // 2. Suscriptor de Detección del LiDAR (Obstáculos)
  lidar_sub_ = this->create_subscription<std_msgs::msg::Bool>(
    "obstacle_detected", 10, [this](const std_msgs::msg::Bool::SharedPtr msg) {
      std::lock_guard<std::mutex> lock(data_mutex_);
      obstacle_detected_ = msg->data;
    });

  // 3. Publicador de Velocidad (Hacia Zenoh -> ESP32)
  cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

  // 4. Servidor de Acción (Escucha al Mission Manager)
  action_server_ = rclcpp_action::create_server<NavigateToPose>(
    this,
    "navigate_to_pose",
    std::bind(&ControllerNode::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
    std::bind(&ControllerNode::handle_cancel, this, std::placeholders::_1),
    std::bind(&ControllerNode::handle_accepted, this, std::placeholders::_1));

  RCLCPP_INFO(this->get_logger(), "👨‍🍳 Chef Controlador EN LÍNEA. Movimiento continuo activado.");
}

double ControllerNode::normalize_angle(double angle) {
  while (angle > M_PI) angle -= 2.0 * M_PI;
  while (angle < -M_PI) angle += 2.0 * M_PI;
  return angle;
}

rclcpp_action::GoalResponse ControllerNode::handle_goal(
  const rclcpp_action::GoalUUID & /*uuid*/, // uuid comentado para evitar warnings de variable sin usar
  std::shared_ptr<const NavigateToPose::Goal> goal)
{
  RCLCPP_INFO(this->get_logger(), "Orden recibida -> Destino: (%.2f, %.2f)", 
              goal->pose.pose.position.x, goal->pose.pose.position.y);
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse ControllerNode::handle_cancel(
  const std::shared_ptr<GoalHandleNavigate> /*goal_handle*/)
{
  RCLCPP_INFO(this->get_logger(), "Orden cancelada. Frenando robot.");
  publish_stop();
  return rclcpp_action::CancelResponse::ACCEPT;
}

void ControllerNode::handle_accepted(const std::shared_ptr<GoalHandleNavigate> goal_handle)
{
  // Ejecutamos en un hilo separado
  std::thread{std::bind(&ControllerNode::execute, this, std::placeholders::_1), goal_handle}.detach();
}

void ControllerNode::publish_stop() {
  auto stop_msg = geometry_msgs::msg::Twist();
  stop_msg.linear.x = 0.0;
  stop_msg.angular.z = 0.0;
  cmd_vel_pub_->publish(stop_msg);
}

void ControllerNode::execute(const std::shared_ptr<GoalHandleNavigate> goal_handle)
{
  auto goal = goal_handle->get_goal();
  double target_x = goal->pose.pose.position.x;
  double target_y = goal->pose.pose.position.y;
  
  auto feedback = std::make_shared<NavigateToPose::Feedback>();
  auto result = std::make_shared<NavigateToPose::Result>();

  rclcpp::Rate loop_rate(20); // Bucle a 20Hz (50ms) para fluidez

  while (rclcpp::ok()) {
    // 1. Verificar si nos han cancelado
    if (goal_handle->is_canceling()) {
      goal_handle->canceled(result);
      return;
    }

    // 2. Esperar a tener al menos un dato de odometría
    if (!has_odom_) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Esperando datos de /odom...");
      loop_rate.sleep();
      continue;
    }

    // Copiar estado actual de forma segura
    double rx, ry, ryaw;
    bool is_blocked;
    {
      std::lock_guard<std::mutex> lock(data_mutex_);
      rx = current_x_;
      ry = current_y_;
      ryaw = current_yaw_;
      is_blocked = obstacle_detected_;
    }

    // 3. Evaluar Obstáculo (LiDAR)
    if (is_blocked) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "¡OBSTÁCULO! Esperando...");
      publish_stop();
      loop_rate.sleep();
      continue; // Saltamos la lógica de movimiento pero no abortamos la misión
    }

    // 4. Matemáticas de Control Cinemático
    double dx = target_x - rx;
    double dy = target_y - ry;
    double distance = std::hypot(dx, dy);

    // ¡AQUÍ ESTÁ LA MAGIA DEL MOVIMIENTO CONTINUO!
    if (distance <= ACCEPTANCE_RADIUS) {
      goal_handle->succeed(result);
      RCLCPP_INFO(this->get_logger(), "Waypoint alcanzado (flujo continuo).");
      return;
    }

    // Calcular error angular
    double target_yaw = std::atan2(dy, dx);
    double error_yaw = normalize_angle(target_yaw - ryaw);

    // Calcular velocidades usando Proporcional
    double v = Kp_linear * distance;
    double w = Kp_angular * error_yaw;

    // Suavizador de curva
    v = v * std::max(0.0, 1.0 - std::abs(error_yaw) / M_PI);

    // Limitar a velocidades máximas del hardware
    v = std::clamp(v, -MAX_LINEAR_SPEED, MAX_LINEAR_SPEED);
    w = std::clamp(w, -MAX_ANGULAR_SPEED, MAX_ANGULAR_SPEED);

    // 5. Publicar Comando
    auto twist = geometry_msgs::msg::Twist();
    twist.linear.x = v;
    twist.angular.z = w;
    cmd_vel_pub_->publish(twist);

    // Feedback al Capitán
    feedback->distance_remaining = distance;
    goal_handle->publish_feedback(feedback);

    loop_rate.sleep();
  }
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ControllerNode>());
  rclcpp::shutdown();
  return 0;
}