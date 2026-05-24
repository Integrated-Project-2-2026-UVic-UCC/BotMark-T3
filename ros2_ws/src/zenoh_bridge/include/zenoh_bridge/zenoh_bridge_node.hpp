#pragma once

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "geometry_msgs/msg/twist.hpp" // Añadido para recibir Twist
#include <zenoh.h>
#include <tf2/LinearMath/Quaternion.h>

#pragma pack(push, 1) // Crucial para que PC y ESP32 coincidan en tamaño
struct CommandData {
    float linear_x;
    float angular_z;
    bool emergency_stop;
};

struct SensorData {
    int32_t ticks_izquierdo;
    int32_t ticks_derecho;
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};
#pragma pack(pop)

class ZenohBridgeNode : public rclcpp::Node {
public:
    ZenohBridgeNode();
    virtual ~ZenohBridgeNode();

private:
    void init_zenoh();
    static void zenoh_sensor_callback(struct z_loaned_sample_t* sample, void* arg);
    
    // --- NUEVO MÉTODO DE ODOMETRÍA ---
    void publish_real_odometry(int32_t current_ticks_left, int32_t current_ticks_right);

    // Recursos de ROS 2
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_; // Suscriptor de Twist

    // Recursos de Zenoh
    z_owned_session_t z_session_;
    z_owned_subscriber_t z_sub_;
    z_owned_publisher_t z_cmd_pub_; // Publicador hacia el ESP32

    // --- VARIABLES DE ODOMETRÍA ---
    // ¡OJO! Cambia estos valores por las medidas reales de tu robot
    const double WHEEL_RADIUS = 0.033;   // Radio de la rueda en metros (ej: 3.3 cm)
    const double WHEEL_BASE = 0.160;     // Distancia entre ruedas en metros (ej: 16 cm)
    const double TICKS_PER_REV = 360.0;  // Ticks que da tu encoder en 1 vuelta completa

    int32_t last_ticks_l_ = 0;
    int32_t last_ticks_r_ = 0;
    bool first_reading_ = true;

    double x_ = 0.0;
    double y_ = 0.0;
    double theta_ = 0.0;
};