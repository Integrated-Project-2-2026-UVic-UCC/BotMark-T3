// This program performs a motor testing routine to verify movement accuracy 
// by traveling a target distance of 0.5 meters using encoder feedback.

#include <Arduino.h>

#include <motors_controller.h>
#include <encoder_handler.h>

#include "config.h"

// Hardware pin mapping for motor control
MotorHW hw_left = {Pin::M_LEFT_IN1, Pin::M_LEFT_IN2, Pin::M_LEFT_PWM};
MotorHW hw_right = {Pin::M_RIGHT_IN1, Pin::M_RIGHT_IN2, Pin::M_RIGHT_PWM};

// Encoder pin configuration
EncoderConfig enc_left = {Pin::ENC_LEFT_A, Pin::ENC_LEFT_B};
EncoderConfig enc_right = {Pin::ENC_RIGHT_A, Pin::ENC_RIGHT_B};

// Robot mechanical specifications for odometry calculations
RobotPhysics robot_physics = {Phys::WHEEL_DIAMETER, Phys::PPR, Phys::GEAR_RATIO}; 

// Motor and encoder control instances
MotorsController motorsController(hw_left, hw_right);
EncoderHandler encoderHandler(enc_right, enc_left, robot_physics);

unsigned long last_control_time = 0;

void setup() {
    Serial.begin(115200);
    
    // Initialize motor and encoder subsystems
    motorsController.begin();
    encoderHandler.begin();

    Serial.println("Distance Test: Target 0.5 meters");
    delay(2000); 
}

void loop() {
    unsigned long current_time = millis();
    float delta_time = (current_time - last_control_time) / 1000.0;
    // Process current encoder data
    encoderHandler.update(delta_time);

    // Retrieve movement metrics
    float current_distance = encoderHandler.getDistRight(); 
    float current_velocity = encoderHandler.getVelocityRight();

    // Output status to serial monitor
    Serial.print("Dist: "); Serial.print(current_distance);
    Serial.print(" m | Vel: "); Serial.print(current_velocity);
    Serial.println(" m/s");

    // Movement logic to reach target distance
    if (current_distance < 0.50) {
        // Apply 50% duty cycle to move forward
        motorsController.move(128, 128);
    } else {
        // Stop motors and enter infinite loop upon reaching target
        motorsController.move(0, 0);
        Serial.println("Target Reached");
        while(1); 
    }
    last_control_time = current_time;
    // Stabilize loop execution
    delay(10); 
}