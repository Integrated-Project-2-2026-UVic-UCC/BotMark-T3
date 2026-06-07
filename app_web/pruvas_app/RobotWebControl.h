#ifndef ROBOT_WEB_CONTROL_H
#define ROBOT_WEB_CONTROL_H

#include <WiFi.h>
#include <WebServer.h>

// Type definition for callback functions
typedef void (*CommandCallback)();

class RobotWebControl {
public:
    // Constructor
    RobotWebControl(int port = 80);
    
    // Connect to WiFi and start the server
    void begin(const char* ssid, const char* password);
    
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

#endif
