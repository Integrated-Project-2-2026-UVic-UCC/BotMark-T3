// Implementation of the WebAppController library for managing Wi-Fi connectivity
// and hosting an HTTP server to process remote control commands.

#include "web_app_controller.h"

WebAppController::WebAppController(int port) : _server(port) {
    _start_callback = nullptr;
    _stop_callback = nullptr;
    _pause_callback = nullptr;
    _resume_callback = nullptr;
}

void WebAppController::enableCors() {
    // Attach standard Cross-Origin Resource Sharing headers to allow frontend communication
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void WebAppController::handleOptions() {
    // Acknowledge preflight requests with a 204 No Content status
    enableCors();
    _server.send(204);
}

void WebAppController::begin(String ssid, String password) {
    // Establish Wi-Fi connection if not already connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting to WiFi");
        WiFi.begin(ssid, password);
        
        while (WiFi.status() != WL_CONNECTED) { 
            delay(500); 
            Serial.print("."); 
        }
        
        Serial.println("\nWiFi Connected");
        delay(2000); 
    }
    
    // Output assigned network IP for client access
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Register endpoints for CORS preflight validation
    _server.on("/status", HTTP_OPTIONS, [this]() { handleOptions(); });
    _server.on("/start", HTTP_OPTIONS, [this]() { handleOptions(); });
    _server.on("/stop", HTTP_OPTIONS, [this]() { handleOptions(); });
    _server.on("/pause", HTTP_OPTIONS, [this]() { handleOptions(); });
    _server.on("/resume", HTTP_OPTIONS, [this]() { handleOptions(); });

    // Endpoint to verify active server connection status
    _server.on("/status", HTTP_GET, [this]() {
        enableCors();
        _server.send(200, "text/plain", "OK");
    });

    // Endpoint to trigger the autonomous operation start sequence
    _server.on("/start", HTTP_POST, [this]() {
        enableCors();
        if (_start_callback) _start_callback();
        _server.send(200, "text/plain", "Started");
    });

    // Endpoint to trigger emergency stop and halt all movement
    _server.on("/stop", HTTP_POST, [this]() {
        enableCors();
        if (_stop_callback) _stop_callback();
        _server.send(200, "text/plain", "Stopped");
    });

    // Endpoint to temporarily suspend operations
    _server.on("/pause", HTTP_POST, [this]() {
        enableCors();
        if (_pause_callback) _pause_callback();
        _server.send(200, "text/plain", "Paused");
    });

    // Endpoint to resume operations from a paused state
    _server.on("/resume", HTTP_POST, [this]() {
        enableCors();
        if (_resume_callback) _resume_callback();
        _server.send(200, "text/plain", "Resumed");
    });

    // Initialize the HTTP listener on the specified port
    _server.begin();
    Serial.println("HTTP server started");
}

void WebAppController::handleClient() {
    // Process incoming client requests synchronously
    _server.handleClient();
}

void WebAppController::onStart(CommandCallback callback) {
    _start_callback = callback;
}

void WebAppController::onStop(CommandCallback callback) {
    _stop_callback = callback;
}

void WebAppController::onPause(CommandCallback callback) {
    _pause_callback = callback;
}

void WebAppController::onResume(CommandCallback callback) {
    _resume_callback = callback;
}