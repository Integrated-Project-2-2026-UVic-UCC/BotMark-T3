// This program performs a PID controller calibration for a single motor 
// using Teleplot to visualize the target velocity, actual velocity, and PWM output.

#include <Arduino.h>

#include <motors_controller.h>
#include <encoder_handler.h>
#include <pid_controller.h>

#include "config.h"

// Hardware and physical parameters setup
MotorHW hw_left = {Pin::M_LEFT_IN1, Pin::M_LEFT_IN2, Pin::M_LEFT_PWM};
MotorHW hw_right = {Pin::M_RIGHT_IN1, Pin::M_RIGHT_IN2, Pin::M_RIGHT_PWM};

EncoderConfig enc_left = {Pin::ENC_LEFT_A, Pin::ENC_LEFT_B};
EncoderConfig enc_right = {Pin::ENC_RIGHT_A, Pin::ENC_RIGHT_B};

RobotPhysics robot_physics = {Phys::WHEEL_DIAMETER, Phys::PPR, Phys::GEAR_RATIO}; 

// Motor and encoder control instances
MotorsController motorsController(hw_left, hw_right);
EncoderHandler encoderHandler(enc_right, enc_left, robot_physics);

// PID configuration for the left wheel
PIDController pid_left(PID::LEFT_KP, PID::LEFT_KI, PID::LEFT_KD, PID::LEFT_MIN, PID::LEFT_MAX);

unsigned long last_time = 0;
float target_velocity = 0.05; // Target velocity set to 0.05 m/s for testing

void setup() {
    Serial.begin(115200);
    
    // Initialize motor and encoder subsystems
    motorsController.begin();
    encoderHandler.begin();
    
    Serial.println("PID Calibration Mode");
}

void loop() {
    unsigned long current_time = millis();
    float delta_time = (current_time - last_time) / 1000.0;
    
    // Execute control loop at 50 Hz (every 0.02 seconds)
    if (delta_time >= 0.02) { 
        // Process current encoder data
        encoderHandler.update(delta_time);
        float actual_velocity = encoderHandler.getVelocityLeft();
        
        // Compute PID output and apply to the left motor only
        int pwm_output = (int)pid_left.linealCompute(target_velocity, actual_velocity, delta_time);
        motorsController.move(pwm_output, 0);
        
        // Teleplot formatting for serial visualization
        Serial.print(">target:"); Serial.println(target_velocity);
        Serial.print(">actual:"); Serial.println(actual_velocity);
        Serial.print(">pwm:"); Serial.println(pwm_output);
        
        last_time = current_time;
    }
}