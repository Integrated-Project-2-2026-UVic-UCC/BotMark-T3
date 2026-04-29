#pragma once

#include <Arduino.h>

#include <Arduino.h>

struct WheelSpeeds {
    float left;  // m/s
    float right; // m/s
};

struct OdometryData {
    float v;     // Velocidad lineal real (m/s)
    float w;     // Velocidad angular real (rad/s)
};

class Kinematics {
private:
    float _trackWidth;

public:
    Kinematics(float trackWidth);

    // Cinemática Inversa: Para mover los motores
    WheelSpeeds calculateWheelSpeeds(float v, float w);

    // Cinemática Directa: Para enviar a ROS 2
    // En lugar de X e Y, calculamos las velocidades actuales
    OdometryData calculateOdometry(float velIzq, float velDer);
};