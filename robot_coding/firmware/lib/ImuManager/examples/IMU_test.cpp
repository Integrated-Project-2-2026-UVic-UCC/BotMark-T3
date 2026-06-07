// This program performs an IMU testing routine to initialize the ICM-20948 sensor,
// calibrate the initial yaw orientation, and stream processed angular data.

#include <Arduino.h>
#include <imu_manager.h>
#include "config.h"

// Global instance for the IMU manager
IMUManager imuManager;

// Variable to control print interval without blocking the CPU
unsigned long last_print_time = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing ICM-20948 IMU Test");

    // Initialize and verify sensor connection via I2C
    if (!imuManager.begin(Pin::IMU_SDA, Pin::IMU_SCL)) {
        Serial.println("Error: Could not find the ICM-20948. Check the wiring (0x68).");
        while (1); // Halt execution if connection fails
    }

    Serial.println("IMU initialized. Keep the robot steady to calibrate zero heading...");
    delay(2000); 
    
    // Reset heading to define the initial forward orientation
    imuManager.resetYaw();
    Serial.println("Yaw reset to 0. You may now rotate the sensor.");
}

void loop() {
    // Read raw sensor data and perform mathematical calculations
    imuManager.update();

    // Output data to serial monitor at 10Hz to prevent CPU saturation
    if (millis() - last_print_time > 100) {
        
        // Retrieve processed yaw value in radians (-PI to PI)
        float current_yaw_rad = imuManager.getYawRad();
        
        Serial.print("Yaw (Rad): ");
        Serial.print(current_yaw_rad, 4);
        Serial.print(" | Yaw (Deg): ");
        Serial.println(current_yaw_rad * (180.0f / PI));

        last_print_time = millis();
    }
}