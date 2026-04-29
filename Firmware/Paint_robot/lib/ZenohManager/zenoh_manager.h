#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include "zenoh-pico.h"

// Estructura para recibir órdenes de ROS 2 (cmd_vel)
struct TwistCommand {
    float linear_x;
    float angular_z;
};

class ZenohManager {
private:
    z_session_t session;
    const char* _expr_pub = "rt/robot/status"; // Tópico de salida
    const char* _expr_sub = "rt/robot/cmd_vel"; // Tópico de entrada
    
    bool _connected = false;
    TwistCommand _last_command = {0.0f, 0.0f};

public:
    ZenohManager();
    
    // Conecta al WiFi y luego al Router de Zenoh
    bool begin(const char* ssid, const char* password, const char* router_ip);
    
    // Mantiene viva la conexión (llamar en cada loop)
    void update();

    // Publica los datos del robot (v, w, yaw)
    void publishStatus(float v, float w, float yaw);

    // Obtener el último comando recibido
    TwistCommand getLastCommand();
    
    bool isConnected();

    // Callback interno para cuando llega un mensaje
    static void on_data(const z_sample_t* sample, void* arg);
};