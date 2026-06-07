// Header file defining the ControllerNode node for handling ROS 2 navigation actions,
// integrating odometry, obstacle detection, and issuing velocity commands.

#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <cmath>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp> 
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <std_msgs/msg/bool.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

using NavigateToPose = nav2_msgs::action::NavigateToPose;
using GoalHandleNavigate = rclcpp_action::ServerGoalHandle<NavigateToPose>;

class ControllerNode : public rclcpp::Node
{
public:
    ControllerNode();

private:
    // Operational state flags
    bool _is_paused = false;
    bool _is_stopped = true;

    // ROS 2 subscriptions, publishers, and action server
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr _pause_sub;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr _stop_sub;
    rclcpp_action::Server<NavigateToPose>::SharedPtr _action_server;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr _odom_sub;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr _lidar_sub;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr _cmd_vel_pub;

    // Internal state variables
    std::mutex _data_mutex; 
    double _current_x = 0.0;
    double _current_y = 0.0;
    double _current_yaw = 0.0;
    bool _has_odom = false;
    bool _obstacle_detected = false;

    // Kinematic control parameters and tolerances
    const double ACCEPTANCE_RADIUS = 0.25; 
    const double MAX_LINEAR_SPEED = 0.4;   
    const double MAX_ANGULAR_SPEED = 1.0;

    const double KP_LINEAR = 0.5;          
    const double KP_ANGULAR = 1.5;

    double _integral_error = 0.0;
    const double KI_LINEAR = 0.05;
    const double INTEGRAL_LIMIT = 0.2; 


    // Private utility and action handling methods
    double normalizeAngle(double angle);

    rclcpp_action::GoalResponse handleGoal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const NavigateToPose::Goal> goal);

    rclcpp_action::CancelResponse handleCancel(
        const std::shared_ptr<GoalHandleNavigate> goal_handle);

    void handleAccepted(const std::shared_ptr<GoalHandleNavigate> goal_handle);

    void publishStop();

    void execute(const std::shared_ptr<GoalHandleNavigate> goal_handle);
};