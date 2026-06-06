#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include <WebServer.h>

// Type definition for callback functions
typedef void (*CommandCallback)();

class WebAppController {
public:
    // Constructor
    WebAppController(int port = 80);
    
    // Connect to WiFi and start the server
    void begin(String ssid = "", String password = "");
    
    // Call this inside your loop()
    void handleClient();
    
    // Methods to attach your custom robot functions
    void onStart(CommandCallback callback);
    void onStop(CommandCallback callback);
    void onPause(CommandCallback callback);
    void onResume(CommandCallback callback);

private:
    WebServer server;
    
    // Stored callbacks
    CommandCallback startCallback;
    CommandCallback stopCallback;
    CommandCallback pauseCallback;
    CommandCallback resumeCallback;

    // Helper functions for CORS
    void enableCORS();
    void handleOptions();
};
