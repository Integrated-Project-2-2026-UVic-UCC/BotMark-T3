#pragma once

#include <Arduino.h>

class PIDController {
private:
    float _kp, _ki, _kd;
    float _minOut, _maxOut;
    float _errorAcumulado;
    float _ultimoError;
    unsigned long _ultimoTiempo;

public:
    // Constructor
    PIDController(float kp, float ki, float kd, float minOut, float maxOut);
    
    // Función principal que usarás en el loop
    float compute(float setpoint, float valorActual);
    
    // Función para resetear la memoria del PID si el robot se detiene
    void reset();
};