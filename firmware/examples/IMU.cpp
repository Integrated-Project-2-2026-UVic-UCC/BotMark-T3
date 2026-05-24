#include <Arduino.h>
#include "imu_manager.h"

// Instancia global de tu gestor de IMU
IMUManager imu;

// Variable para controlar el tiempo de impresión sin bloquear la CPU
unsigned long lastPrint = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("--- Iniciando Prueba de IMU ICM-20948 ---");

    // NOTA: Ya no llamamos a Wire.begin() aquí porque tu método 
    // imu.begin() ya se encarga de abrir el puerto I2C en los pines 21 y 22.
    
    // Iniciar y verificar la conexión
    if (!imu.begin()) {
        Serial.println("Error: No se pudo encontrar la ICM-20948. Revisa el cableado (0x68).");
        while (1); // Detener ejecución si hay fallo físico
    }

    Serial.println("IMU iniciada. Mantén el robot quieto para fijar el cero inicial...");
    delay(2000); // Pequeña pausa para estabilizar lecturas
    
    // Fijar el "Frente" del robot
    imu.resetYaw();
    Serial.println("Yaw reseteado a 0. ¡Ya puedes girar el sensor!");
}

void loop() {
    // 1. Leer los datos crudos y calcular las matemáticas (atan2, normalización)
    imu.update();

    // 2. Imprimir datos cada 100ms (10 veces por segundo)
    if (millis() - lastPrint > 100) {
        
        // Obtener el valor ya procesado (-PI a PI)
        float yaw = imu.getYawRad();
        
        Serial.print("Yaw (Rad): ");
        Serial.print(yaw, 4);
        Serial.print(" | Yaw (Deg): ");
        Serial.println(yaw * (180.0f / PI));

        lastPrint = millis();
    }
}