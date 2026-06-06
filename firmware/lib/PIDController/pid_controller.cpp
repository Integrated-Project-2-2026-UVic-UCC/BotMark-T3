#include "pid_controller.h"

PIDController::PIDController(float kp, float ki, float kd, float minOut, float maxOut, float initialAngle) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _minOut = minOut;
    _maxOut = maxOut;
    _targetAngle = initialAngle;
    _errorAcumulado = 0.0;
    _ultimoError = 0.0;
}

float PIDController::linealcompute(float setpoint, float valorActual, float deltaTime) {
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

    // Salida total mapeada y limitada al PWM permitido
    float salida = P + I + D;
    return constrain(salida, _minOut, _maxOut);
}

float PIDController::circularcompute(float targetRate, float valorActual, float deltaTime) {
    // Prevenir división por cero si el bucle va demasiado rápido
    if (deltaTime <= 0.0) return 0.0; 

    // 1. Avanzar el target si estamos haciendo una curva
    _targetAngle += targetRate * deltaTime;
    _targetAngle = atan2(sin(_targetAngle), cos(_targetAngle));

    // 2. Calcular el error por el camino más corto (vital para evitar tirones en PI y -PI)
    float error = atan2(sin(_targetAngle - valorActual), cos(_targetAngle - valorActual));

    // Proporcional
    float P = _kp * error;

    // Integral (con anti-windup dinámico)
    _errorAcumulado += error * deltaTime;
    // Limitamos la acumulación de la integral (ajusta estos valores según tu hardware)
    _errorAcumulado = constrain(_errorAcumulado, -2.0, 2.0); 
    float I = _ki * _errorAcumulado;

    // Derivativo
    float D = _kd * ((error - _ultimoError) / deltaTime);

    // Guardar memoria para la siguiente vuelta
    _ultimoError = error;

    // Salida total mapeada y limitada al PWM permitido
    float salida = P + I + D;
    return constrain(salida, _minOut, _maxOut);
}

void PIDController::reset(float currentAngle) {
    _errorAcumulado = 0.0;
    _ultimoError = 0.0;
    _targetAngle = currentAngle;
}