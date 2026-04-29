#include "kinematics.h"

Kinematics::Kinematics(float trackWidth) : _trackWidth(trackWidth) {}

// CINEMÁTICA INVERSA: Twist -> Ruedas
WheelSpeeds Kinematics::calculateWheelSpeeds(float v, float w) {
    WheelSpeeds ws;
    ws.left  = v - (w * _trackWidth / 2.0f);
    ws.right = v + (w * _trackWidth / 2.0f);
    return ws;
}

// CINEMÁTICA DIRECTA: Ruedas -> Velocidades del Robot
// Esto es lo que enviarás en el mensaje 'nav_msgs/Odometry' o 'geometry_msgs/Twist' hacia ROS 2
OdometryData Kinematics::calculateOdometry(float velIzq, float velDer) {
    OdometryData odom;
    // La velocidad lineal es el promedio de ambas ruedas
    odom.v = (velDer + velIzq) / 2.0f;
    // La velocidad angular es la diferencia dividida por la distancia entre ruedas
    odom.w = (velDer - velIzq) / _trackWidth;
    
    return odom;
}