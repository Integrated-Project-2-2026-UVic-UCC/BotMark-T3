#include "motors_controller.h"

MotorsController::MotorsController(MotorHW hwA, MotorHW hwB): 
    _AIN1(hwA.IN1), _AIN2(hwA.IN2), _PWMA(hwA.PWM), 
    _BIN1(hwB.IN1), _BIN2(hwB.IN2), _PWMB(hwB.PWM) {}

void MotorsController::begin() {
    pinMode(_AIN1, OUTPUT);
    pinMode(_AIN2, OUTPUT);  
    pinMode(_BIN1, OUTPUT);
    pinMode(_BIN2, OUTPUT);

    // Configurar el canal PWM para el motor A y B
    // Canal 0 -> generador de pwm que tiene la esp32. por eso se usa ledc en vez de analogwrite. hay de 0-15 canales.
    // 20kHz -> Es lo suficientemente alto para ser inaudible i lo suficientemente bajo para no generar demasiado calor por pérdidas de conmutación en el driver i ser fino.
    // 8 bits -> define tus pasos de velocidad. Con 8 bits, tienes 256 niveles (de 0 a 255) de sobras.
    ledcAttach(_PWMA, 20000, 8); 
    ledcAttach(_PWMB, 20000, 8);
}

void MotorsController::move(int velA, int velB) {
    apply_speed_limits(velA, velB);

    setMotor(_AIN1, _AIN2, _PWMA, velA); 
    setMotor(_BIN1, _BIN2, _PWMB, -velB); 
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Private

void MotorsController::apply_speed_limits(int &velA, int &velB) {
    int max_speed = max(abs(velA), abs(velB));
    if (max_speed > 255) {
        float scaling_factor = 255.0 / max_speed;
        velA = (int)(velA * scaling_factor);
        velB = (int)(velB * scaling_factor);
    }
}

void MotorsController::setMotor(int IN1, int IN2, int pin_pwm, int speed) {
    // Escribimos en los pines directamente el resultado de la comparación
    digitalWrite(IN1, speed > 0); 
    digitalWrite(IN2, speed < 0);
    ledcWrite(pin_pwm, abs(speed));
}
