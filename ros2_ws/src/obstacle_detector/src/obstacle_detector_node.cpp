#include "obstacle_detector/obstacle_detector_node.hpp" // Ajusta la ruta si está dentro de una subcarpeta en include

ObstacleDetector::ObstacleDetector() : Node("obstacle_detector") {
    
    // Parámetro de distancia (0.4m por defecto)
    this->declare_parameter("detection_distance", 0.4);

    this->xvel_ = 0.0;
    this->yvel_ = 0.0;
    this->phi_ = 0.0;
    
    this->obstacle_status_.data = false;
    this->threshold_distance_ = this->get_parameter("detection_distance").as_double();

    // Publicador hacia el controlador
    this->publisher_ = this->create_publisher<std_msgs::msg::Bool>("obstacle_detected", 10);

    // Suscriptor del LiDAR
    this->scan_subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "/scan", 10, std::bind(&ObstacleDetector::scan_callback, this, std::placeholders::_1));

    // Suscriptor de velocidades
    this->twist_subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10, std::bind(&ObstacleDetector::twist_callback, this, std::placeholders::_1));
        
    RCLCPP_INFO(this->get_logger(), "🛡️ Radar Direccional Activado. Distancia: %.2f metros.", threshold_distance_);
}

void ObstacleDetector::twist_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    this->xvel_ = msg->linear.x;
    this->yvel_ = msg->linear.y;
}

void ObstacleDetector::scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    
    // Si el robot se mueve, calculamos hacia dónde (ángulo local de movimiento)
    if (std::abs(this->xvel_) > 0.001 || std::abs(this->yvel_) > 0.001) {
        this->phi_ = std::atan2(this->yvel_, this->xvel_);
    }
    
    // Cono de visión: 45 grados a la izquierda y derecha de la dirección de avance
    double angle_min_rad = (-45.0 * M_PI / 180.0) + this->phi_;
    double angle_max_rad = (45.0 * M_PI / 180.0) + this->phi_;

    // Normalización circular entre 0 y 2*PI
    angle_min_rad = std::fmod(angle_min_rad, 2.0 * M_PI);
    if (angle_min_rad < 0) angle_min_rad += 2.0 * M_PI;

    angle_max_rad = std::fmod(angle_max_rad, 2.0 * M_PI);
    if (angle_max_rad < 0) angle_max_rad += 2.0 * M_PI;

    // Calcular los índices del array del LiDAR que corresponden a esos ángulos
    int index_min = static_cast<int>(angle_min_rad / msg->angle_increment);
    int index_max = static_cast<int>(angle_max_rad / msg->angle_increment);

    bool detected = false;
    int num_readings = msg->ranges.size();

    // Función lambda para comprobar si un rayo concreto detecta un obstáculo
    auto check_obstacle = [&](int index) {
        if (index < 0 || index >= num_readings) return false;

        float r = msg->ranges[index];
        return (!std::isnan(r) && !std::isinf(r) && r < this->threshold_distance_);
    };

    // Recorrido del cono de visión
    if (index_min <= index_max) {
        for (int i = index_min; i < index_max; ++i) {
            if (check_obstacle(i)) { detected = true; break; }
        }
    } else {
        // Caso circular: el cono cruza el ángulo 0 (ej. pasa de 350º a 10º)
        for (int i = index_min; i < num_readings; ++i) {
            if (check_obstacle(i)) { detected = true; break; }
        }
        if (!detected) {
            for (int i = 0; i < index_max; ++i) {
                if (check_obstacle(i)) { detected = true; break; }
            }
        }
    }

    // Publicamos el estado (True = obstáculo, False = vía libre)
    this->obstacle_status_.data = detected;
    this->publisher_->publish(this->obstacle_status_);

    if (detected) {
        RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
            "¡Obstáculo detectado en la trayectoria! Frenando...");
    }
}

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObstacleDetector>());
    rclcpp::shutdown();
    return 0;
}