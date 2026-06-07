// This program tests the web application controller by establishing a Wi-Fi connection 
// and setting up a server to handle incoming HTTP requests for robot control.

#include <Arduino.h>
#include <web_app_controller.h>
#include "secrets.h"

// Network credentials configuration
const char* WIFI_SSID = Network::SSID;
const char* WIFI_PASSWORD = Network::PASSWORD;

// Web server controller instance bound to standard HTTP port 80
WebAppController webAppServer(80);

// Callback functions executed upon receiving specific HTTP requests
void HandleStop() {
    Serial.println("STOP command received.");
}

void HandleStart() {
    Serial.println("START command received.");
}

void HandlePause() {
    Serial.println("PAUSE command received.");
}

void HandleResume() {
    Serial.println("RESUME command received.");
}

void setup() {
    Serial.begin(115200);
    delay(1000); 

    // Bind callback functions to the respective web server endpoints
    webAppServer.onStop(HandleStop);
    webAppServer.onStart(HandleStart);
    webAppServer.onPause(HandlePause);
    webAppServer.onResume(HandleResume);

    // Initialize network connection and launch the web server
    webAppServer.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    // Continuously process incoming client requests
    webAppServer.handleClient();
    
    // Stabilize loop execution
    delay(10); 
}