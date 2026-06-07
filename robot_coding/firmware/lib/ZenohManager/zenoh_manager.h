// Header file defining the ZenohManager library for handling Zenoh-Pico network 
// communication, including publisher and subscriber node declarations.

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "zenoh-pico.h"

#pragma pack(push, 1)

struct Twist {
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

class ZenohManager {
public:
    ZenohManager();
    
    bool begin(const char* router_ip, String ssid = "", String password = "");
    
    // Publishing Methods
    void publishSensors(const SensorData& data);
    void publishPaused(bool paused);
    void publishStopped(bool stopped);
    
    // Query Methods
    Twist getLastCommand();
    bool isObstacleDetected();
    bool isConnected();

    // Static Callbacks for Zenoh-Pico
    static void onTwist(z_loaned_sample_t* sample, void* arg);
    static void onObstacle(z_loaned_sample_t* sample, void* arg);

private:
    z_owned_session_t _session;
    
    // Subscribers 
    z_owned_subscriber_t _sub_cmd;      
    z_owned_subscriber_t _sub_obstacle; 

    // Publishers 
    z_owned_publisher_t _pub_sensors;   
    z_owned_publisher_t _pub_paused;    
    z_owned_publisher_t _pub_stopped;    
    
    // Key Expressions (Values kept exactly as requested)
    const char* EXPR_PUB_SENSORS  = "rt/robot/sensors";
    const char* EXPR_SUB_CMD      = "rt/robot/twist";
    const char* EXPR_PUB_PAUSED   = "rt/robot/is_paused";
    const char* EXPR_PUB_STOPPED  = "rt/robot/is_stoped";
    const char* EXPR_SUB_OBSTACLE = "rt/robot/obstacle_detected";
    
    bool _connected = false;
    Twist _last_command = {0.0f, 0.0f};
    bool _obstacle_detected = false;
};