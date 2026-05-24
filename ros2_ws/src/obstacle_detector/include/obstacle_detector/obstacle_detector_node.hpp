#pragma once

#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

class ObstacleDetector : public rclcpp::Node {
public:
    ObstacleDetector();

private:
    // Callbacks
    void twist_callback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg);

    // Variables de estado
    double xvel_;
    double yvel_;
    double phi_;
    double threshold_distance_;
    std_msgs::msg::Bool obstacle_status_;

    // Interfaces de ROS 2
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_subscription_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr twist_subscription_;
};