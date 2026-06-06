#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "zenoh-pico.h"

#pragma pack(push, 1)
// Estructura actualizada sin emergency_stop
struct Twist {
    float linear_x;
    float angular_z;
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
    
    // Suscriptores (ESP32 escucha)
    z_owned_subscriber_t sub_cmd;      // Recibe Twist
    z_owned_subscriber_t sub_obstacle; // Recibe obstáculo

    // Publicadores (ESP32 habla)
    z_owned_publisher_t pub_sensors;   // Envía Odometría e IMU
    z_owned_publisher_t pub_paused;    // Envía estado Pausa
    z_owned_publisher_t pub_stoped;    // Envía estado Stop
    
    // Rutas (Key Expressions)
    const char* _expr_pub_sensors  = "rt/robot/sensores"; 
    const char* _expr_sub_cmd      = "rt/robot/twsit"; 
    const char* _expr_pub_paused   = "rt/robot/is_paused";
    const char* _expr_pub_stoped   = "rt/robot/is_stoped";
    const char* _expr_sub_obstacle = "rt/robot/obstacle_detected";
    
    bool _connected = false;
    Twist _last_command = {0.0f, 0.0f};
    bool _obstacle_detected = false;

public:
    ZenohManager();
    
    bool begin(const char* router_ip, String ssid = "", String password = "");
    
    // Métodos de publicación
    void publishSensors(const SensorData& data);
    void publishPaused(bool paused);
    void publishStopped(bool stopped);
    
    // Métodos de consulta
    Twist getLastCommand();
    bool isObstacleDetected();
    bool isConnected();

    // Callbacks estáticos para Zenoh-Pico
    static void on_twist(z_loaned_sample_t* sample, void* arg);
    static void on_obstacle(z_loaned_sample_t* sample, void* arg);
};