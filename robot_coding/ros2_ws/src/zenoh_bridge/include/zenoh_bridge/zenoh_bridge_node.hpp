// Header file defining the ZenohBridgeNode node for routing data between ROS 2 topics 
// and Zenoh key expressions, including odometry calculations.

#pragma once

#include <zenoh.h>
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <geometry_msgs/msg/twist.hpp> 
#include <std_msgs/msg/bool.hpp>
#include <tf2/LinearMath/Quaternion.h>

#pragma pack(push, 1) // Ensures strict byte alignment between PC and ESP32
struct CommandData {
    float linear_x;
    float angular_z;
};

struct SensorData {
    int32_t ticks_left;
    int32_t ticks_right;
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
    void initZenoh();
    static void zenohSensorCallback(struct z_loaned_sample_t* sample, void* arg);
    void publishRealOdometry(int32_t current_ticks_left, int32_t current_ticks_right);

    // ROS 2 Interfaces
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr _is_paused_pub;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr _is_stopped_pub;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr _odom_pub;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr _imu_pub;

    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr _obstacle_sub;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr _cmd_vel_sub; 

    // Zenoh Interfaces
    z_owned_session_t _z_session;
    z_owned_subscriber_t _z_sub_sensors;
    z_owned_publisher_t _z_cmd_pub; 
    z_owned_subscriber_t _z_sub_paused;
    z_owned_subscriber_t _z_sub_stopped;
    z_owned_publisher_t _z_pub_obstacle;

    static void zenohPausedCallback(struct z_loaned_sample_t* sample, void* arg);
    static void zenohStoppedCallback(struct z_loaned_sample_t* sample, void* arg);

    // Odometry physical parameters
    const double WHEEL_RADIUS = 0.08;   
    const double WHEEL_BASE = 0.235;     
    const double TICKS_PER_REV = 7700.0;  

    int32_t _last_ticks_l = 0;
    int32_t _last_ticks_r = 0;
    bool _first_reading = true;

    double _x = 0.0;
    double _y = 0.0;
    double _theta = 0.0;
};