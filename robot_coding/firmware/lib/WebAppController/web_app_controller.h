// Header file defining the WebAppController library for managing Wi-Fi connectivity
// and hosting an HTTP server to process remote control commands.

#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Function pointer definition for HTTP command execution callbacks
typedef void (*CommandCallback)();

class WebAppController {
public:
    // Constructor initializes the HTTP server on the specified network port
    WebAppController(int port = 80);
    
    // Establishes Wi-Fi connection and initializes the HTTP listener
    void begin(String ssid = "", String password = "");
    
    // Synchronously processes incoming HTTP client requests
    void handleClient();
    
    // Callback registration methods for web interface commands
    void onStart(CommandCallback callback);
    void onStop(CommandCallback callback);
    void onPause(CommandCallback callback);
    void onResume(CommandCallback callback);

private:
    // Internal web server instance
    WebServer _server;
    
    // Registered callback function pointers for execution state management
    CommandCallback _start_callback;
    CommandCallback _stop_callback;
    CommandCallback _pause_callback;
    CommandCallback _resume_callback;

    // Internal helper methods for managing Cross-Origin Resource Sharing
    void enableCors();
    void handleOptions();
};