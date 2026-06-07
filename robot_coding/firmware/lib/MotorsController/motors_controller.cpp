// Implementation of the MotorsController library for driving dual DC motors,
// managing PWM channels, and applying intelligent speed scaling.

#include "motors_controller.h"

MotorsController::MotorsController(MotorHW hwA, MotorHW hwB): 
    _AIN1(hwA.IN1), _AIN2(hwA.IN2), _PWMA(hwA.PWM), 
    _BIN1(hwB.IN1), _BIN2(hwB.IN2), _PWMB(hwB.PWM) {}

void MotorsController::begin() {
    pinMode(_AIN1, OUTPUT);
    pinMode(_AIN2, OUTPUT);  
    pinMode(_BIN1, OUTPUT);
    pinMode(_BIN2, OUTPUT);

    // Configure the PWM channel for Motor A and Motor B
    // Channel 0 -> ESP32 hardware PWM generator. There are 16 channels (0-15).
    // Frequency -> Balances audible noise and switching losses in the driver.
    // 8-bit resolution -> Defines speed steps, providing 256 levels (0 to 255).
    
    // Channel 0 assigned to Motor A
    ledcSetup(0, 5000, 8); 
    ledcAttachPin(_PWMA, 0);

    // Channel 1 assigned to Motor B
    ledcSetup(1, 5000, 8); 
    ledcAttachPin(_PWMB, 1);
}

void MotorsController::move(int velA, int velB) {
    applySpeedLimits(velA, velB);

    setMotor(_AIN1, _AIN2, 0, -velA); 
    setMotor(_BIN1, _BIN2, 1, velB); 
}

void MotorsController::applySpeedLimits(int &velA, int &velB) {
    int max_speed = max(abs(velA), abs(velB));
    if (max_speed > 255) {
        float scaling_factor = 255.0 / max_speed;
        velA = (int)(velA * scaling_factor);
        velB = (int)(velB * scaling_factor);
    }
}

void MotorsController::setMotor(int IN1, int IN2, int channel_pwm, int speed) {
    // Write logic states directly to the direction pins based on speed polarity
    digitalWrite(IN1, speed > 0); 
    digitalWrite(IN2, speed < 0);
    
    // Output the absolute speed value to the PWM channel
    ledcWrite(channel_pwm, abs(speed));
}