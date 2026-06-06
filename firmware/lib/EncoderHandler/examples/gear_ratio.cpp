// This program performs a motor testing routine to calibrate encoder ticks 
// by executing a brief movement and reporting the resulting encoder counts.

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

void setup() {
    Serial.begin(115200);
    
    // Initialize motor and encoder subsystems
    motorsController.begin();
    encoderHandler.begin();
    
    delay(3000);

    // Drive motors at full power for a fixed duration to test response
    motorsController.move(255, 255); 
    delay(2000);
    motorsController.move(0, 0);

    // Update encoder state after movement
    encoderHandler.update();
    
    // Output captured tick counts to serial monitor for calibration
    Serial.print("Ticks Left: "); Serial.println(encoderHandler.getTicksLeft());
    Serial.print("Ticks Right: "); Serial.println(encoderHandler.getTicksRight());
}

void loop() {
    // Empty loop as test routine runs only once in setup
}