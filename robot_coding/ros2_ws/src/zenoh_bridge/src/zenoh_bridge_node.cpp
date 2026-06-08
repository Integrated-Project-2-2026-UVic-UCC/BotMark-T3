// Implementation of the ZenohBridgeNode node for routing data between ROS 2 topics 
// and Zenoh key expressions, including odometry calculations.

#include "zenoh_bridge/zenoh_bridge_node.hpp"

#include <cstring> 
#include <cmath>

ZenohBridgeNode::ZenohBridgeNode() : Node("zenoh_bridge_node") {
    _odom_pub = this->create_publisher<nav_msgs::msg::Odometry>("odom_raw", 10);
    _imu_pub = this->create_publisher<sensor_msgs::msg::Imu>("imu_raw", 10);
    _is_paused_pub = this->create_publisher<std_msgs::msg::Bool>("is_paused", 10);
    _is_stopped_pub = this->create_publisher<std_msgs::msg::Bool>("is_stoped", 10);

    initZenoh();
}

ZenohBridgeNode::~ZenohBridgeNode() { 
    z_undeclare_publisher(z_publisher_move(&_z_cmd_pub)); 
    z_undeclare_publisher(z_publisher_move(&_z_pub_obstacle));
    z_undeclare_subscriber(z_subscriber_move(&_z_sub_sensors));
    z_undeclare_subscriber(z_subscriber_move(&_z_sub_paused));
    z_undeclare_subscriber(z_subscriber_move(&_z_sub_stopped));
    z_close((z_loaned_session_t*)z_session_loan(&_z_session), NULL);
    RCLCPP_INFO(this->get_logger(), "Zenoh session closed correctly.");
}

void ZenohBridgeNode::initZenoh() {
    z_owned_config_t config;
    z_config_default(&config);
    zc_config_insert_json5(z_config_loan_mut(&config), "mode", "\"client\"");
    zc_config_insert_json5(z_config_loan_mut(&config), "connect", "[\"tcp/192.168.1.51:7447\"]");

    RCLCPP_INFO(this->get_logger(), "Opening Zenoh session in Client mode...");

    if (z_open(&_z_session, z_config_move(&config), NULL) != Z_OK) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open Zenoh session.");
        return;
    }

    // ESP32 -> PC: Sensor Subscriber
    z_owned_closure_sample_t callback;
    z_closure_sample(&callback, ZenohBridgeNode::zenohSensorCallback, NULL, (void*)this);

    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str(&ke, "rt/robot/sensors");

    if (z_declare_subscriber(z_session_loan(&_z_session), &_z_sub_sensors, z_view_keyexpr_loan(&ke), z_closure_sample_move(&callback), NULL) != Z_OK) {
        RCLCPP_ERROR(this->get_logger(), "Error declaring sensor subscriber in Zenoh.");
    } else {
        RCLCPP_INFO(this->get_logger(), "Zenoh Bridge with Integrated Odometry ready.");
    }

    // PC -> ESP32: Twist Command Publisher
    z_view_keyexpr_t ke_cmd;
    z_view_keyexpr_from_str(&ke_cmd, "rt/robot/twist"); 
    z_declare_publisher(z_session_loan(&_z_session), &_z_cmd_pub, z_view_keyexpr_loan(&ke_cmd), NULL);

    _cmd_vel_sub = this->create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 
        10, 
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
            CommandData cmd;
            cmd.linear_x = static_cast<float>(msg->linear.x);
            cmd.angular_z = static_cast<float>(msg->angular.z);

            z_owned_bytes_t payload;
            z_bytes_copy_from_buf(&payload, (const uint8_t*)&cmd, sizeof(CommandData));
            
            if (z_publisher_put(z_publisher_loan(&_z_cmd_pub), z_bytes_move(&payload), NULL) != Z_OK) {
                RCLCPP_ERROR(this->get_logger(), "Error sending command via Zenoh.");
            }
        }
    );

    // ESP32 -> PC: Pause and Stop Subscribers
    z_owned_closure_sample_t cb_paused;
    z_closure_sample(&cb_paused, ZenohBridgeNode::zenohPausedCallback, NULL, (void*)this);
    z_view_keyexpr_t ke_paused;
    z_view_keyexpr_from_str(&ke_paused, "rt/robot/is_paused");
    z_declare_subscriber(z_session_loan(&_z_session), &_z_sub_paused, z_view_keyexpr_loan(&ke_paused), z_closure_sample_move(&cb_paused), NULL);

    z_owned_closure_sample_t cb_stopped;
    z_closure_sample(&cb_stopped, ZenohBridgeNode::zenohStoppedCallback, NULL, (void*)this);
    z_view_keyexpr_t ke_stopped;
    z_view_keyexpr_from_str(&ke_stopped, "rt/robot/is_stoped");
    z_declare_subscriber(z_session_loan(&_z_session), &_z_sub_stopped, z_view_keyexpr_loan(&ke_stopped), z_closure_sample_move(&cb_stopped), NULL);

    // PC -> ESP32: Obstacle Publisher
    z_view_keyexpr_t ke_obs;
    z_view_keyexpr_from_str(&ke_obs, "rt/robot/obstacle_detected");
    z_declare_publisher(z_session_loan(&_z_session), &_z_pub_obstacle, z_view_keyexpr_loan(&ke_obs), NULL);

    // PC -> ESP32: ROS 2 Subscriber listening to Obstacle topic to forward via Zenoh
    _obstacle_sub = this->create_subscription<std_msgs::msg::Bool>(
        "obstacle_detected", 
        10, 
        [this](const std_msgs::msg::Bool::SharedPtr msg) {
            bool is_obstacle = msg->data;
            z_owned_bytes_t payload;
            z_bytes_copy_from_buf(&payload, (const uint8_t*)&is_obstacle, sizeof(bool));
            
            if (z_publisher_put(z_publisher_loan(&_z_pub_obstacle), z_bytes_move(&payload), NULL) != Z_OK) {
                RCLCPP_ERROR(this->get_logger(), "Error sending obstacle state via Zenoh.");
            }
        }
    );
}

void ZenohBridgeNode::publishRealOdometry(int32_t current_ticks_left, int32_t current_ticks_right) {
    if (_first_reading) {
        _last_ticks_l = current_ticks_left;
        _last_ticks_r = current_ticks_right;
        _first_reading = false;
        return;
    }

    int32_t delta_ticks_l = current_ticks_left - _last_ticks_l;
    int32_t delta_ticks_r = current_ticks_right - _last_ticks_r;

    _last_ticks_l = current_ticks_left;
    _last_ticks_r = current_ticks_right;

    double dist_l = (2.0 * M_PI * WHEEL_RADIUS) * (static_cast<double>(delta_ticks_l) / TICKS_PER_REV);
    double dist_r = (2.0 * M_PI * WHEEL_RADIUS) * (static_cast<double>(delta_ticks_r) / TICKS_PER_REV);

    double dist_c = (dist_r + dist_l) / 2.0;
    double delta_theta = (dist_r - dist_l) / WHEEL_BASE;

    _x += dist_c * cos(_theta + (delta_theta / 2.0));
    _y += dist_c * sin(_theta + (delta_theta / 2.0));
    _theta += delta_theta;

    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, _theta);

    auto odom_msg = nav_msgs::msg::Odometry();
    odom_msg.header.stamp = this->now();
    odom_msg.header.frame_id = "odom";
    odom_msg.child_frame_id = "base_footprint";

    odom_msg.pose.pose.position.x = _x;
    odom_msg.pose.pose.position.y = _y;
    odom_msg.pose.pose.position.z = 0.0;
    odom_msg.pose.pose.orientation.x = q.x();
    odom_msg.pose.pose.orientation.y = q.y();
    odom_msg.pose.pose.orientation.z = q.z();
    odom_msg.pose.pose.orientation.w = q.w();

    odom_msg.twist.twist.linear.x = dist_c / 0.1; 
    odom_msg.twist.twist.angular.z = delta_theta / 0.1;

    _odom_pub->publish(odom_msg);
}

void ZenohBridgeNode::zenohSensorCallback(struct z_loaned_sample_t* sample, void* arg) {
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

        auto imu_msg = sensor_msgs::msg::Imu();
        imu_msg.header.stamp = node->now();
        imu_msg.header.frame_id = "imu_link";
        imu_msg.linear_acceleration.x = data.accel_x;
        imu_msg.linear_acceleration.y = data.accel_y;
        imu_msg.linear_acceleration.z = data.accel_z;
        imu_msg.angular_velocity.x = data.gyro_x;
        imu_msg.angular_velocity.y = data.gyro_y;
        imu_msg.angular_velocity.z = data.gyro_z;
        node->_imu_pub->publish(imu_msg);

        node->publishRealOdometry(data.ticks_left, data.ticks_right);
        RCLCPP_INFO_THROTTLE(node->get_logger(), *node->get_clock(), 1000, "Received: TicksL=%d TicksR=%d", data.ticks_left, data.ticks_right);
        
    } else {
        RCLCPP_ERROR(node->get_logger(), "Corrupted data size received.");
    }
}

void ZenohBridgeNode::zenohPausedCallback(struct z_loaned_sample_t* sample, void* arg) {
    ZenohBridgeNode* node = static_cast<ZenohBridgeNode*>(arg);
    const struct z_loaned_bytes_t* payload = z_sample_payload(sample);
    
    if (z_bytes_len(payload) == sizeof(bool)) {
        bool val;
        z_owned_slice_t slice;
        z_bytes_to_slice(payload, &slice);
        std::memcpy(&val, z_slice_data(z_slice_loan(&slice)), sizeof(bool));
        z_slice_drop(z_slice_move(&slice));

        auto msg = std_msgs::msg::Bool();
        msg.data = val;
        node->_is_paused_pub->publish(msg);
    }
}

void ZenohBridgeNode::zenohStoppedCallback(struct z_loaned_sample_t* sample, void* arg) {
    ZenohBridgeNode* node = static_cast<ZenohBridgeNode*>(arg);
    const struct z_loaned_bytes_t* payload = z_sample_payload(sample);
    
    if (z_bytes_len(payload) == sizeof(bool)) {
        bool val;
        z_owned_slice_t slice;
        z_bytes_to_slice(payload, &slice);
        std::memcpy(&val, z_slice_data(z_slice_loan(&slice)), sizeof(bool));
        z_slice_drop(z_slice_move(&slice));

        auto msg = std_msgs::msg::Bool();
        msg.data = val;
        node->_is_stopped_pub->publish(msg);
    }
}

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ZenohBridgeNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}