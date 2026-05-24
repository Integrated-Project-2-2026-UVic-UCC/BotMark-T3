#pragma once

#include <Arduino.h>

struct WheelSpeeds {
    float left;  // m/s
    float right; // m/s
};

class Kinematics {
private:
    float _trackWidth;

public:
    Kinematics(float trackWidth);

    // Cinemática Inversa: Para mover los motores
    WheelSpeeds calculateWheelSpeeds(float v, float w);
};