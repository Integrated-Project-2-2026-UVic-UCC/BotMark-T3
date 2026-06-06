#include <Arduino.h>

#include "secrets.h"

#include "zenoh_manager.h"

// Configuración de red
const char* ssid = Network::SSID;
const char* pass = Network::PASSWORD;
const char* router_ip = Network::ROUTER_IP; // La IP del ordenador en la red

ZenohManager zm;

void setup() {
    Serial.begin(115200);

    // Inicializamos el manager
    if (zm.begin(router_ip, ssid, pass)) {
        Serial.println("¡Zenoh Manager iniciado correctamente!");
    } else {
        Serial.println("Error al iniciar Zenoh.");
    }
}

void loop() {
    if (zm.isConnected()) {
        // Simulamos datos de sensores
        SensorData datos;
        datos.ticks_izquierdo = millis() / 100;
        datos.ticks_derecho = millis() / 100;
        datos.accel_x = 0.0;
        datos.accel_y = 0.0;
        datos.accel_z = 9.8;
        datos.gyro_x = 0.0;
        datos.gyro_y = 0.0;
        datos.gyro_z = 0.0;

        // Publicamos
        zm.publishSensors(datos);
        
        Serial.println("Datos enviados por Zenoh...");
    }
    
    delay(100); // Enviamos a 10Hz
}