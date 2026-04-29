#include "pid_controller.h"

PIDController::PIDController(float kp, float ki, float kd, float minOut, float maxOut) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _minOut = minOut;
    _maxOut = maxOut;
    _errorAcumulado = 0.0;
    _ultimoError = 0.0;
    _ultimoTiempo = millis();
}

float PIDController::compute(float setpoint, float valorActual) {
    unsigned long tiempoActual = millis();
    float deltaTime = (tiempoActual - _ultimoTiempo) / 1000.0; // Convertir a segundos

    // Prevenir división por cero si el bucle va demasiado rápido
    if (deltaTime <= 0.0) return 0.0; 

    float error = setpoint - valorActual;

    // Proporcional
    float P = _kp * error;

    // Integral (con anti-windup dinámico)
    _errorAcumulado += error * deltaTime;
    // Limitamos la acumulación de la integral (ajusta estos valores según tu hardware)
    _errorAcumulado = constrain(_errorAcumulado, -50.0, 50.0); 
    float I = _ki * _errorAcumulado;

    // Derivativo
    float D = _kd * ((error - _ultimoError) / deltaTime);

    // Guardar memoria para la siguiente vuelta
    _ultimoError = error;
    _ultimoTiempo = tiempoActual;

    // Salida total mapeada y limitada al PWM permitido
    float salida = P + I + D;
    return constrain(salida, _minOut, _maxOut);
}

void PIDController::reset() {
    _errorAcumulado = 0.0;
    _ultimoError = 0.0;
    _ultimoTiempo = millis();
}