#include <Arduino.h>

#include "secrets.h"
#include "web_app_controller.h"


// 1. Configura tus credenciales Wi-Fi
const char* ssid = Network::SSID;
const char* password = Network::PASSWORD;

// 2. Instanciar la clase pasando el puerto (el 80 es el estándar para HTTP)
WebAppController servidorRobot(80);

// Estas funciones se ejecutan automáticamente cuando llega una petición HTTP
void handleStop() {
    Serial.println("\n[!] Comando STOP recibido.");
}

void handleStart() {
    Serial.println("\n[>] Comando START recibido.");
}

void handlePause() {
    Serial.println("\n[!] Comando PAUSE recibido.");
}

void handleResume() {
    Serial.println("\n[>] Comando RESUME recibido.");
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Pequeña pausa para que el monitor serie se estabilice

    // 3. Vincular las funciones a tu librería
    servidorRobot.onStop(handleStop);
    servidorRobot.onStart(handleStart);
    servidorRobot.onPause(handlePause);
    servidorRobot.onResume(handleResume);

    // 4. Iniciar la conexión Wi-Fi y el servidor
    servidorRobot.begin(ssid, password);
}

void loop() {
    // 5. Escuchar peticiones de la aplicación web de forma continua
    servidorRobot.handleClient();
    delay(10); // Pequeña pausa para evitar saturar el loop
}