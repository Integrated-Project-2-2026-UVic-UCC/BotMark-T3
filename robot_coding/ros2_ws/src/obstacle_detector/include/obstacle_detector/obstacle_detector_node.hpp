// Header file defining the ObstacleDetector node to evaluate LiDAR point clouds 
// dynamically based on the current velocity vector to trigger safety stops.

#pragma once

#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

class ObstacleDetector : public rclcpp::Node {
public:
    ObstacleDetector();

private:
    // Callbacks
    void TwistCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void ScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);

    // State variables
    double _x_vel;
    double _y_vel;
    double _phi;
    double _threshold_distance;
    std_msgs::msg::Bool _obstacle_status;

    // ROS 2 Interfaces
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr _publisher;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr _scan_subscription;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr _twist_subscription;
};