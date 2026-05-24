#include "zenoh_bridge/zenoh_bridge_node.hpp"
#include <cstring> 
#include <cmath>

ZenohBridgeNode::ZenohBridgeNode() : Node("zenoh_bridge_node") {
    // Ahora publicamos en el topic oficial de odometría
    odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom_raw", 10);
    imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu_raw", 10);

    init_zenoh();
}

ZenohBridgeNode::~ZenohBridgeNode() {
    z_undeclare_publisher(z_publisher_move(&z_cmd_pub_)); // Limpiamos el nuevo publicador
    z_undeclare_subscriber(z_subscriber_move(&z_sub_));
    z_close((z_loaned_session_t*)z_session_loan(&z_session_), NULL);
    RCLCPP_INFO(this->get_logger(), "Sesión de Zenoh cerrada correctamente.");
}

void ZenohBridgeNode::init_zenoh() {
    z_owned_config_t config;
    z_config_default(&config);
    zc_config_insert_json5(z_config_loan_mut(&config), "mode", "\"client\"");
    zc_config_insert_json5(z_config_loan_mut(&config), "connect", "[\"tcp/192.168.1.51:7447\"]");

    RCLCPP_INFO(this->get_logger(), "Abriendo sesión de Zenoh en modo Cliente...");

    if (z_open(&z_session_, z_config_move(&config), NULL) != Z_OK) {
        RCLCPP_ERROR(this->get_logger(), "No se pudo abrir la sesión de Zenoh.");
        return;
    }

    // --- SUSCRIPTOR ORIGINAL (ESP32 -> PC) ---
    z_owned_closure_sample_t callback;
    z_closure_sample(&callback, ZenohBridgeNode::zenoh_sensor_callback, NULL, (void*)this);

    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str(&ke, "rt/robot/sensores");

    if (z_declare_subscriber(z_session_loan(&z_session_), &z_sub_, z_view_keyexpr_loan(&ke), z_closure_sample_move(&callback), NULL) != Z_OK) {
        RCLCPP_ERROR(this->get_logger(), "Error al declarar el suscriptor en Zenoh.");
    } else {
        RCLCPP_INFO(this->get_logger(), "Puente Zenoh con Odometría Integrada listo.");
    }

    // --- NUEVO: PUBLICADOR ZENOH Y SUSCRIPTOR ROS 2 (PC -> ESP32) ---
    z_view_keyexpr_t ke_cmd;
    z_view_keyexpr_from_str(&ke_cmd, "rt/robot/comandos");
    z_declare_publisher(z_session_loan(&z_session_), &z_cmd_pub_, z_view_keyexpr_loan(&ke_cmd), NULL);

    cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 
        10, 
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
            CommandData cmd;
            cmd.linear_x = static_cast<float>(msg->linear.x);
            cmd.angular_z = static_cast<float>(msg->angular.z);
            cmd.emergency_stop = false; 

            z_owned_bytes_t payload;
            z_bytes_copy_from_buf(&payload, (const uint8_t*)&cmd, sizeof(CommandData));
            RCLCPP_WARN(this->get_logger(), "REcibed command via ROS 2.");
            
            if (z_publisher_put(z_publisher_loan(&z_cmd_pub_), z_bytes_move(&payload), NULL) != Z_OK) {
                RCLCPP_WARN(this->get_logger(), "Error enviando comando por Zenoh.");
            }
        }
    );
}

// --- NUEVO CÁLCULO DIRECTO DE ODOMETRÍA ---
void ZenohBridgeNode::publish_real_odometry(int32_t current_ticks_left, int32_t current_ticks_right) {
    if (first_reading_) {
        last_ticks_l_ = current_ticks_left;
        last_ticks_r_ = current_ticks_right;
        first_reading_ = false;
        return;
    }

    // Calcular la diferencia de ticks
    int32_t delta_ticks_l = current_ticks_left - last_ticks_l_;
    int32_t delta_ticks_r = current_ticks_right - last_ticks_r_;

    // Actualizar historial
    last_ticks_l_ = current_ticks_left;
    last_ticks_r_ = current_ticks_right;

    // Convertir ticks a distancia en metros
    double dist_l = (2.0 * M_PI * WHEEL_RADIUS) * (static_cast<double>(delta_ticks_l) / TICKS_PER_REV);
    double dist_r = (2.0 * M_PI * WHEEL_RADIUS) * (static_cast<double>(delta_ticks_r) / TICKS_PER_REV);

    // Cinemática de tracción diferencial
    double dist_c = (dist_r + dist_l) / 2.0;
    double delta_theta = (dist_r - dist_l) / WHEEL_BASE;

    // Actualizar posición global (x, y, theta)
    x_ += dist_c * cos(theta_ + (delta_theta / 2.0));
    y_ += dist_c * sin(theta_ + (delta_theta / 2.0));
    theta_ += delta_theta;

    // Convertir theta a Cuaternión de ROS 2
    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, theta_);

    // Crear y rellenar el mensaje oficial
    auto odom_msg = nav_msgs::msg::Odometry();
    odom_msg.header.stamp = this->now();
    odom_msg.header.frame_id = "odom";
    odom_msg.child_frame_id = "base_footprint";

    odom_msg.pose.pose.position.x = x_;
    odom_msg.pose.pose.position.y = y_;
    odom_msg.pose.pose.position.z = 0.0;
    odom_msg.pose.pose.orientation.x = q.x();
    odom_msg.pose.pose.orientation.y = q.y();
    odom_msg.pose.pose.orientation.z = q.z();
    odom_msg.pose.pose.orientation.w = q.w();

    // Velocidades estimadas (opcional, calculadas de forma simple asumiendo 10Hz fijos)
    odom_msg.twist.twist.linear.x = dist_c / 0.1; 
    odom_msg.twist.twist.angular.z = delta_theta / 0.1;

    odom_pub_->publish(odom_msg);
}

void ZenohBridgeNode::zenoh_sensor_callback(struct z_loaned_sample_t* sample, void* arg) {
    ZenohBridgeNode* node = static_cast<ZenohBridgeNode*>(arg);
    
    const struct z_loaned_bytes_t* payload = z_sample_payload(sample);
    size_t len = z_bytes_len(payload);

    if (len == sizeof(SensorData)) {
        SensorData data;
        z_owned_slice_t slice;
        z_bytes_to_slice(payload, &slice);
        const uint8_t* raw_data = z_slice_data(z_slice_loan(&slice));
        std::memcpy(&data, raw_data, len);
        z_slice_drop(z_slice_move(&slice));

        // Publicar IMU
        auto imu_msg = sensor_msgs::msg::Imu();
        imu_msg.header.stamp = node->now();
        imu_msg.header.frame_id = "imu_link";
        imu_msg.linear_acceleration.x = data.accel_x;
        imu_msg.linear_acceleration.y = data.accel_y;
        imu_msg.linear_acceleration.z = data.accel_z;
        imu_msg.angular_velocity.x = data.gyro_x;
        imu_msg.angular_velocity.y = data.gyro_y;
        imu_msg.angular_velocity.z = data.gyro_z;
        node->imu_pub_->publish(imu_msg);

        // Llamar a nuestra nueva función de odometría directa
        node->publish_real_odometry(data.ticks_izquierdo, data.ticks_derecho);
        RCLCPP_INFO(node->get_logger(), "Recibido: TicksL=%d TicksR=%d", data.ticks_izquierdo, data.ticks_derecho);
        
    } else {
        RCLCPP_WARN(node->get_logger(), "Tamaño de datos corrupto.");
    }
}

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ZenohBridgeNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}