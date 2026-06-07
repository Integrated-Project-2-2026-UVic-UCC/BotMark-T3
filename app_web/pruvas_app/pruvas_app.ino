#include "RobotWebControl.h"

// Initialize the library
RobotWebControl robotControl;

// Replace with your Wi-Fi credentials
const char* ssid = "arbi";
const char* password = "7ofJFqWP";

void setup() {
  Serial.begin(115200);

  // 1. Define what happens when the web app sends a "START" command
  robotControl.onStart([]() {
    Serial.println("Action: Starting Robot Routine...");
    // TODO: Add your motor start code here
  });

  // 2. Define what happens when the web app sends a "STOP" command
  robotControl.onStop([]() {
    Serial.println("Action: Emergency Stop!");
    // TODO: Add your motor stop code here
  });

  // 3. Define what happens when the web app sends a "PAUSE" command
  robotControl.onPause([]() {
    Serial.println("Action: Pausing Robot...");
    // TODO: Add your motor pause code here
  });

  // 4. Define what happens when the web app sends a "RESUME" command
  robotControl.onResume([]() {
    Serial.println("Action: Resuming Robot...");
    // TODO: Add your motor resume code here
  });

  // Start the Wi-Fi and Web Server
  robotControl.begin(ssid, password);
}

void loop() {
  // Listen for incoming web requests (Required to keep the server responsive)
  robotControl.handleClient();
  
  // Other robot logic (sensors, movement, etc.) can go here!
}
