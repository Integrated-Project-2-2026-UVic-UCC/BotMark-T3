// Implementation of the ControllerNode node for handling ROS 2 navigation actions,
// processing odometry, managing safety states, and executing kinematic control.

#include "controller_pkg/controller_node.hpp"

ControllerNode::ControllerNode() : Node("controller_node")
{
    // Initialize odometry subscriber from Zenoh bridge
    _odom_sub = this->create_subscription<nav_msgs::msg::Odometry>(
        "odom_raw", 10, [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
            std::lock_guard<std::mutex> lock(_data_mutex);
            _current_x = msg->pose.pose.position.x;
            _current_y = msg->pose.pose.position.y;

            tf2::Quaternion q(
                msg->pose.pose.orientation.x,
                msg->pose.pose.orientation.y,
                msg->pose.pose.orientation.z,
                msg->pose.pose.orientation.w);
            tf2::Matrix3x3 m(q);
            double roll, pitch;
            m.getRPY(roll, pitch, _current_yaw);
            
            _has_odom = true;
        });

    // Initialize safety and state subscribers (LiDAR, Pause, Stop)
    _lidar_sub = this->create_subscription<std_msgs::msg::Bool>(
        "obstacle_detected", 10, [this](const std_msgs::msg::Bool::SharedPtr msg) {
            std::lock_guard<std::mutex> lock(_data_mutex);
            _obstacle_detected = msg->data;
        });

    _pause_sub = this->create_subscription<std_msgs::msg::Bool>(
        "is_paused", 10, [this](const std_msgs::msg::Bool::SharedPtr msg) {
            std::lock_guard<std::mutex> lock(_data_mutex);
            _is_paused = msg->data;
        });

    _stop_sub = this->create_subscription<std_msgs::msg::Bool>(
        "is_stoped", 10, [this](const std_msgs::msg::Bool::SharedPtr msg) {
            std::lock_guard<std::mutex> lock(_data_mutex);
            _is_stopped = msg->data;
        });

    // Initialize velocity command publisher
    _cmd_vel_pub = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

    // Initialize ROS 2 action server for navigation
    _action_server = rclcpp_action::create_server<NavigateToPose>(
        this,
        "navigate_to_pose",
        std::bind(&ControllerNode::handleGoal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&ControllerNode::handleCancel, this, std::placeholders::_1),
        std::bind(&ControllerNode::handleAccepted, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Controller node ONLINE. Continuous movement activated.");
}

double ControllerNode::normalizeAngle(double angle) {
    while (angle > M_PI) angle -= 2.0 * M_PI;
    while (angle < -M_PI) angle += 2.0 * M_PI;
    return angle;
}

rclcpp_action::GoalResponse ControllerNode::handleGoal(
    const rclcpp_action::GoalUUID & /*uuid*/,
    std::shared_ptr<const NavigateToPose::Goal> goal)
{
    std::lock_guard<std::mutex> lock(_data_mutex);
    
    // Reject new missions if the robot is in a stopped state
    if (_is_stopped) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Robot in STOP state. Goal rejected.");
        return rclcpp_action::GoalResponse::REJECT;
    }

    RCLCPP_INFO(this->get_logger(), "Goal received Target: (%.2f, %.2f)", 
                goal->pose.pose.position.x, goal->pose.pose.position.y);
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse ControllerNode::handleCancel(
    const std::shared_ptr<GoalHandleNavigate> /*goal_handle*/)
{
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Goal canceled. Stopping robot.");
    publishStop();
    return rclcpp_action::CancelResponse::ACCEPT;
}

void ControllerNode::handleAccepted(const std::shared_ptr<GoalHandleNavigate> goal_handle)
{
    std::thread{std::bind(&ControllerNode::execute, this, std::placeholders::_1), goal_handle}.detach();
}

void ControllerNode::publishStop() {
    auto stop_msg = geometry_msgs::msg::Twist();
    stop_msg.linear.x = 0.0;
    stop_msg.angular.z = 0.0;
    _cmd_vel_pub->publish(stop_msg);
}

void ControllerNode::execute(const std::shared_ptr<GoalHandleNavigate> goal_handle)
{
    auto goal = goal_handle->get_goal();
    double target_x = goal->pose.pose.position.x;
    double target_y = goal->pose.pose.position.y;
    
    auto feedback = std::make_shared<NavigateToPose::Feedback>();
    auto result = std::make_shared<NavigateToPose::Result>();

    rclcpp::Rate loop_rate(20); 

    _integral_error = 0.0;
    
    while (rclcpp::ok()) {
        if (goal_handle->is_canceling()) {
            goal_handle->canceled(result);
            return;
        }

        if (!_has_odom) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000, "Waiting for /odom data...");
            loop_rate.sleep();
            continue;
        }

        // Safely copy current state variables
        double current_x, current_y, current_yaw;
        bool is_blocked, is_paused, is_stopped;
        {
            std::lock_guard<std::mutex> lock(_data_mutex);
            current_x = _current_x;
            current_y = _current_y;
            current_yaw = _current_yaw;
            is_blocked = _obstacle_detected;
            is_paused = _is_paused;
            is_stopped = _is_stopped;
        }

        // Evaluate emergency stop condition
        if (is_stopped) {
            RCLCPP_ERROR(this->get_logger(), "STOP STATE ACTIVATED! Aborting current mission.");
            publishStop();
            goal_handle->abort(result); 
            return; 
        }

        // Evaluate obstacle detection and pause conditions
        if (is_blocked || is_paused) {
            if (is_blocked) {
                RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "OBSTACLE DETECTED! Waiting...");
            } else {
                RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "PAUSE ACTIVATED. Waiting for resumption...");
            }
            publishStop();
            loop_rate.sleep();
            continue; 
        }

        // Calculate the Euclidean distance to the target position
        double delta_x = target_x - current_x;
        double delta_y = target_y - current_y;
        double distance = std::hypot(delta_x, delta_y); 

        // Terminate execution successfully if the robot is within the acceptable tolerance radius
        if (distance <= ACCEPTANCE_RADIUS) {
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Waypoint reached (continuous flow).");
            return;
        }

        // Calculate the shortest path angular error towards the target destination
        double target_yaw = std::atan2(delta_y, delta_x);
        double error_yaw = normalizeAngle(target_yaw - current_yaw);

        // Accumulate distance error over time for the integral term and apply anti-windup limits
        _integral_error += distance * (1.0 / 20.0); 
        _integral_error = std::clamp(_integral_error, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);

        // Compute proportional-integral linear velocity and proportional angular velocity
        double velocity_linear = (KP_LINEAR * distance) + (KI_LINEAR * _integral_error);
        double velocity_angular = KP_ANGULAR * error_yaw;

        // Reduce linear velocity proportionally when angular misalignment is high
        // Clamp final velocities to defined safety limits
        velocity_linear = velocity_linear * std::max(0.0, 1.0 - std::abs(error_yaw) / M_PI);
        velocity_linear = std::clamp(velocity_linear, -MAX_LINEAR_SPEED, MAX_LINEAR_SPEED);
        velocity_angular = std::clamp(velocity_angular, -MAX_ANGULAR_SPEED, MAX_ANGULAR_SPEED);

        // Construct and publish the command velocity message
        auto twist_msg = geometry_msgs::msg::Twist();
        twist_msg.linear.x = velocity_linear;
        twist_msg.angular.z = velocity_angular;
        _cmd_vel_pub->publish(twist_msg);

        // Publish action server feedback
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