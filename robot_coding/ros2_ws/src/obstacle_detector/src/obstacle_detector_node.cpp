// Implementation of the ObstacleDetector node to evaluate LiDAR point clouds 
// dynamically based on the current velocity vector to trigger safety stops.

#include "obstacle_detector/obstacle_detector_node.hpp" 

ObstacleDetector::ObstacleDetector() : Node("obstacle_detector") {
    
    // Distance parameter (0.4m by default)
    this->declare_parameter("detection_distance", 0.4);

    _x_vel = 0.0;
    _y_vel = 0.0;
    _phi = 0.0;
    
    _obstacle_status.data = false;
    _threshold_distance = this->get_parameter("detection_distance").as_double();

    // Publisher to the controller
    _publisher = this->create_publisher<std_msgs::msg::Bool>("obstacle_detected", 10);

    // LiDAR subscriber
    _scan_subscription = this->create_subscription<sensor_msgs::msg::LaserScan>(
        "/scan", 10, std::bind(&ObstacleDetector::ScanCallback, this, std::placeholders::_1));

    // Velocity subscriber
    _twist_subscription = this->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10, std::bind(&ObstacleDetector::TwistCallback, this, std::placeholders::_1));
        
    RCLCPP_INFO(this->get_logger(), "Directional radar activated. Distance: %.2f meters.", _threshold_distance);
}

void ObstacleDetector::TwistCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    _x_vel = msg->linear.x;
    _y_vel = msg->linear.y;
}

void ObstacleDetector::ScanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    
    // If the robot is moving, calculate the local movement angle
    if (std::abs(_x_vel) > 0.001 || std::abs(_y_vel) > 0.001) {
        _phi = std::atan2(_y_vel, _x_vel);
    }
    
    // Vision cone: 45 degrees to the left and right of the forward direction
    double angle_min_rad = (-45.0 * M_PI / 180.0) + _phi;
    double angle_max_rad = (45.0 * M_PI / 180.0) + _phi;

    // Circular normalization between 0 and 2*PI
    angle_min_rad = std::fmod(angle_min_rad, 2.0 * M_PI);
    if (angle_min_rad < 0) angle_min_rad += 2.0 * M_PI;

    angle_max_rad = std::fmod(angle_max_rad, 2.0 * M_PI);
    if (angle_max_rad < 0) angle_max_rad += 2.0 * M_PI;

    // Calculate the LiDAR array indices corresponding to those angles
    int index_min = static_cast<int>(angle_min_rad / msg->angle_increment);
    int index_max = static_cast<int>(angle_max_rad / msg->angle_increment);

    bool detected = false;
    int num_readings = msg->ranges.size();

    // Lambda function to check if a specific ray detects an obstacle
    auto CheckObstacle = [&](int index) {
        if (index < 0 || index >= num_readings) return false;

        float r = msg->ranges[index];
        return (!std::isnan(r) && !std::isinf(r) && r < _threshold_distance);
    };

    // Traverse the vision cone
    if (index_min <= index_max) {
        for (int i = index_min; i < index_max; ++i) {
            if (CheckObstacle(i)) { detected = true; break; }
        }
    } else {
        // Circular case: the cone crosses the 0 angle (e.g., goes from 350 to 10 degrees)
        for (int i = index_min; i < num_readings; ++i) {
            if (CheckObstacle(i)) { detected = true; break; }
        }
        if (!detected) {
            for (int i = 0; i < index_max; ++i) {
                if (CheckObstacle(i)) { detected = true; break; }
            }
        }
    }

    // Publish the state (True = obstacle, False = clear path)
    _obstacle_status.data = detected;
    _publisher->publish(_obstacle_status);

    if (detected) {
        RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
            "Obstacle detected in trajectory! Braking...");
    }
}

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObstacleDetector>());
    rclcpp::shutdown();
    return 0;
}