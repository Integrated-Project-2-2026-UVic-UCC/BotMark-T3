#include "kinematics.h"

Kinematics::Kinematics(float trackWidth) : _trackWidth(trackWidth) {}

// CINEMÁTICA INVERSA: Twist -> Ruedas
WheelSpeeds Kinematics::calculateWheelSpeeds(float v, float w) {
    WheelSpeeds ws;
    ws.left  = v - (w * _trackWidth / 2.0f);
    ws.right = v + (w * _trackWidth / 2.0f);
    return ws;
}