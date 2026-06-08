// Header file defining the MotorsController library for driving dual DC motors,
// managing PWM channels, and applying intelligent speed scaling.

#pragma once

#include <Arduino.h>

// Hardware pin mapping for a single motor
struct MotorHW {
    int IN1;
    int IN2;
    int PWM;
};

class MotorsController {
public:
    // Constructor initializes hardware mappings
    MotorsController(MotorHW hwA, MotorHW hwB);

    // Configures GPIO pins and hardware PWM channels (ledc)
    void begin();

    // Drives the motors with specified velocities (-255 to 255)
    void move(float velA, float velB);

private:
    // Motor A (Left) pins
    int _AIN1, _AIN2, _PWMA;
    // Motor B (Right) pins
    int _BIN1, _BIN2, _PWMB;

    const float _MAX_SPEED_MPS = 1.5; 
    const int _MAX_PWM = 255;         
    const int _MIN_PWM = 50;         

    // Applies speed scaling to keep PWM values within the 0-255 range
    void applySpeedLimits(int &pwmA, int &pwmB);

    // Executes physical signals to the motor driver hardware
    void setMotor(int IN1, int IN2, int channel_pwm, int speed);
    
};