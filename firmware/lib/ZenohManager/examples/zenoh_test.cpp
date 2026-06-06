// This program tests the Zenoh communication protocol by establishing a connection
// to a network router and publishing simulated sensor data at a fixed frequency.

#include <Arduino.h>
#include <zenoh_manager.h>
#include "secrets.h"

// Network and router configuration constants
const char* WIFI_SSID = Network::SSID;
const char* WIFI_PASSWORD = Network::PASSWORD;
const char* ROUTER_IP = Network::ROUTER_IP; 

// Zenoh communication manager instance
ZenohManager zenoh_manager;

void setup() {
    Serial.begin(115200);

    // Initialize Zenoh manager with network credentials and target router IP
    Serial.println("Initializing Zenoh Manager");
        while (!zenoh_manager.begin(ROUTER_IP, WIFI_SSID, WIFI_PASSWORD)) { 
        Serial.print(".");
        delay(300);
    }
    Serial.println("Zenoh Manager initialized successfully");
}

void loop() {
    // Verify active connection before attempting to publish
    if (zenoh_manager.isConnected()) {
        
        // Populate struct with simulated sensor telemetry
        SensorData sensor_data;
        sensor_data.ticks_izquierdo = millis() / 100;
        sensor_data.ticks_derecho = millis() / 100;
        sensor_data.accel_x = 0.0;
        sensor_data.accel_y = 0.0;
        sensor_data.accel_z = 9.8;
        sensor_data.gyro_x = 0.0;
        sensor_data.gyro_y = 0.0;
        sensor_data.gyro_z = 0.0;

        // Transmit the telemetry data over the Zenoh network
        zenoh_manager.publishSensors(sensor_data);
        
        Serial.println("Data sent via Zenoh");
    }
    
    // Regulate transmission rate to 10Hz
    delay(100); 
}