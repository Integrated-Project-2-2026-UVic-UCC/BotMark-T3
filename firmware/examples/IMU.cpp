#include <Arduino.h>

#include "imu_manager.h"

IMUManager imu;
unsigned long lastPrint = 0;

void setup() {
    // WT901B es la imu vieja, la actual es MPU9250
    Serial.begin(115200);
    Serial.println("--- Iniciando Prueba de IMU ---");

    Wire.begin();
    
    if (!imu.begin()) {
        Serial.println("Error: No se pudo encontrar el MPU9250. Revisa el cableado (0x68).");
        while (1);
    }

    Serial.println("IMU iniciada. Mantén el robot quieto para el offset inicial.");
    delay(2000);
    
    // Seteamos el cero inicial
    imu.resetYaw();
    Serial.println("Yaw reseteado a 0. ¡Ya puedes mover la IMU!");
}

void loop() {
    // Actualizar los filtros internos de la IMU
    imu.update();

    // Imprimir datos cada 100ms para no saturar el puerto serie
    if (millis() - lastPrint > 100) {
        float yaw = imu.getYawRad();
        
        Serial.print("Yaw (Rad): ");
        Serial.print(yaw, 4);
        Serial.print(" | Yaw (Deg): ");
        Serial.println(yaw * (180.0f / PI));

        lastPrint = millis();
    }
}