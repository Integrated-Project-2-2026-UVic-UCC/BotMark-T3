#include <Arduino.h>

#include "config.h"

#include "motors_controller.h"
#include "encoder_handler.h"

// Hardware pin mapping for motor control
MotorHW hw_left = {Pin::M_IZQ_IN1, Pin::M_IZQ_IN2, Pin::M_IZQ_PWM};
MotorHW hw_right = {Pin::M_DER_IN1, Pin::M_DER_IN2, Pin::M_DER_PWM};

// Encoder pin configuration
EncoderConfig enc_left = {Pin::ENC_IZQ_A, Pin::ENC_IZQ_B};
EncoderConfig enc_right = {Pin::ENC_DER_A, Pin::ENC_DER_B};

// Robot mechanical specifications for odometry calculations
RobotPhysics robot_physics = {Phys::WHEEL_DIAMETER, Phys::PPR, Phys::GEAR_RATIO}; 

// Motor and encoder control instances
MotorsController motorsController(hw_left, hw_right);
EncoderHandler encoderHandler(enc_right, enc_left, robot_physics);

void setup() {
    Serial.begin(115200);
    
    // Initialize motor and encoder subsystems
    motorsController.begin();
    encoderHandler.begin();

    Serial.println("Distance Test: Target 0.5 meters");
    delay(2000); 
}

void loop() {
    // Process current encoder data
    encoderHandler.update();

    // Retrieve movement metrics
    float current_distance = encoderHandler.getDistIzq(); 
    float current_velocity = encoderHandler.getVelocityIzq();

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

    // Stabilize loop execution
    delay(10); 
}