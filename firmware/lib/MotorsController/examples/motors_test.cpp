// This program performs a comprehensive testing sequence for the motor control subsystem,
// evaluating progressive acceleration, reversing, pivoting, and intelligent PWM scaling.

#include <Arduino.h>
#include <motors_controller.h>
#include "config.h"

// Hardware pin mapping for motor control
MotorHW hw_left = {Pin::M_LEFT_IN1, Pin::M_LEFT_IN2, Pin::M_LEFT_PWM};
MotorHW hw_right = {Pin::M_RIGHT_IN1, Pin::M_RIGHT_IN2, Pin::M_RIGHT_PWM};

// Motor control instance
MotorsController motorsController(hw_left, hw_right);

void setup() {
    Serial.begin(115200);
    
    // Initialize motor subsystem
    motorsController.begin();
    Serial.println("Motor Test Initiated");
}

void loop() {
    // TEST 1: Progressive forward movement (Velocity ramp)
    Serial.println("Test 1: Progressive forward");
    for (int i = 0; i <= 255; i += 50) {
        motorsController.move(i, i);
        delay(500);
    }
    
    // TEST 2: Reverse
    Serial.println("Test 2: Full reverse");
    motorsController.move(-200, -200);
    delay(2000);

    // TEST 3: Pivot turn
    Serial.println("Test 3: Right turn");
    motorsController.move(150, -150);
    delay(1000);

    // TEST 4: Intelligent scaling verification
    // Sending values above 255 to verify the library scales them correctly
    Serial.println("Test 4: Testing intelligent scaling (>255)");
    motorsController.move(500, 250); 
    delay(2000);

    // Full stop
    Serial.println("Stopping");
    motorsController.move(0, 0);
    delay(3000);
}