#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "std_msgs/msg/bool.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

using NavigateToPose = nav2_msgs::action::NavigateToPose;
using GoalHandleNavigate = rclcpp_action::ServerGoalHandle<NavigateToPose>;

class ControllerNode : public rclcpp::Node
{
public:
  ControllerNode();

private:
  // ROS 2 Interfaces
  rclcpp_action::Server<NavigateToPose>::SharedPtr action_server_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr lidar_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;

  // Variables de estado
  std::mutex data_mutex_; // Protege los datos compartidos entre callbacks y el hilo de ejecución
  double current_x_ = 0.0;
  double current_y_ = 0.0;
  double current_yaw_ = 0.0;
  bool has_odom_ = false;
  bool obstacle_detected_ = false;

  // Parámetros de Control
  const double MAX_LINEAR_SPEED = 0.4;   // m/s
  const double MAX_ANGULAR_SPEED = 1.0;  // rad/s
  const double Kp_linear = 0.5;          // Ganancia proporcional lineal
  const double Kp_angular = 1.5;         // Ganancia proporcional angular
  const double ACCEPTANCE_RADIUS = 0.25; // Distancia para dar el punto por bueno

  // Métodos privados
  double normalize_angle(double angle);

  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const NavigateToPose::Goal> goal);

  rclcpp_action::CancelResponse handle_cancel(
    const std::shared_ptr<GoalHandleNavigate> goal_handle);

  void handle_accepted(const std::shared_ptr<GoalHandleNavigate> goal_handle);

  void publish_stop();

  void execute(const std::shared_ptr<GoalHandleNavigate> goal_handle);
};