#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "zenoh-pico.h"

#pragma pack(push, 1)
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

class ZenohManager {
private:
    z_owned_session_t session;
    z_owned_subscriber_t sub; // Necesitamos guardar el suscriptor
    z_owned_publisher_t pub;

    
    const char* _expr_pub = "rt/robot/sensores"; 
    const char* _expr_sub = "rt/robot/comandos"; 
    
    bool _connected = false;
    CommandData _last_command = {0.0f, 0.0f, false};

public:
    ZenohManager();
    
    bool begin(const char* ssid, const char* password, const char* router_ip);
    void publishSensors(const SensorData& data);
    CommandData getLastCommand();
    bool isConnected();

    // El callback en v1.x usa z_loaned_sample_t
    static void on_data(z_loaned_sample_t* sample, void* arg);
};