#include "web_app_controller.h"

WebAppController::WebAppController(int port) : server(port) {
    startCallback = nullptr;
    stopCallback = nullptr;
    pauseCallback = nullptr;
    resumeCallback = nullptr;
}

void WebAppController::enableCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void WebAppController::handleOptions() {
    enableCORS();
    server.send(204);
}

void WebAppController::begin(String ssid, String password) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Conectando a WiFi");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) { 
            delay(500); 
            Serial.print("."); 
        }
        Serial.println("\nWiFi Conectado!");
        delay(2000); 
    }
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

void WebAppController::handleClient() {
    // Listen for incoming requests from the web app
    server.handleClient();
}

void WebAppController::onStart(CommandCallback callback) {
    startCallback = callback;
}

void WebAppController::onStop(CommandCallback callback) {
    stopCallback = callback;
}

void WebAppController::onPause(CommandCallback callback) {
    pauseCallback = callback;
}

void WebAppController::onResume(CommandCallback callback) {
    resumeCallback = callback;
}
