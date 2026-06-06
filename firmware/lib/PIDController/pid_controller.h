#pragma once

#include <Arduino.h>

class PIDController {
private:
    float _kp, _ki, _kd;
    float _minOut, _maxOut, _targetAngle;
    float _errorAcumulado;
    float _ultimoError;
    unsigned long _ultimoTiempo;

public:
    // Constructor
    PIDController(float kp, float ki, float kd, float minOut, float maxOut, float initialAngle = 0.0);
    
    // Función principal que usarás en el loop
    float linealcompute(float setpoint, float valorActual, float deltaTime);
    float circularcompute(float setpoint, float valorActual, float deltaTime);

    // Función para resetear la memoria del PID si el robot se detiene
    void reset(float currentAngle = 0.0);
};