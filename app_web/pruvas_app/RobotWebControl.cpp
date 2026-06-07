#include "RobotWebControl.h"

RobotWebControl::RobotWebControl(int port) : server(port) {
    startCallback = nullptr;
    stopCallback = nullptr;
    pauseCallback = nullptr;
    resumeCallback = nullptr;
}

void RobotWebControl::enableCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void RobotWebControl::handleOptions() {
    enableCORS();
    server.send(204);
}

void RobotWebControl::begin(const char* ssid, const char* password) {
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup CORS endpoints (preflight requests)
    server.on("/status", HTTP_OPTIONS, [this]() { handleOptions(); });
    server.on("/start", HTTP_OPTIONS, [this]() { handleOptions(); });
    server.on("/stop", HTTP_OPTIONS, [this]() { handleOptions(); });
    server.on("/pause", HTTP_OPTIONS, [this]() { handleOptions(); });
    server.on("/resume", HTTP_OPTIONS, [this]() { handleOptions(); });

    // 1. Status Endpoint (used for connection dot)
    server.on("/status", HTTP_GET, [this]() {
        enableCORS();
        server.send(200, "text/plain", "OK");
    });

    // 2. Start Command
    server.on("/start", HTTP_POST, [this]() {
        enableCORS();
        if (startCallback) startCallback();
        server.send(200, "text/plain", "Started");
    });

    // 3. Stop Command (Emergency Stop)
    server.on("/stop", HTTP_POST, [this]() {
        enableCORS();
        if (stopCallback) stopCallback();
        server.send(200, "text/plain", "Stopped");
    });

    // 4. Pause Command
    server.on("/pause", HTTP_POST, [this]() {
        enableCORS();
        if (pauseCallback) pauseCallback();
        server.send(200, "text/plain", "Paused");
    });

    // 5. Resume Command
    server.on("/resume", HTTP_POST, [this]() {
        enableCORS();
        if (resumeCallback) resumeCallback();
        server.send(200, "text/plain", "Resumed");
    });

    // Start the server
    server.begin();
    Serial.println("HTTP server started");
}

void RobotWebControl::handleClient() {
    // Listen for incoming requests from the web app
    server.handleClient();
}

void RobotWebControl::onStart(CommandCallback callback) {
    startCallback = callback;
}

void RobotWebControl::onStop(CommandCallback callback) {
    stopCallback = callback;
}

void RobotWebControl::onPause(CommandCallback callback) {
    pauseCallback = callback;
}

void RobotWebControl::onResume(CommandCallback callback) {
    resumeCallback = callback;
}
