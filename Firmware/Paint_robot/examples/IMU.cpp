#include <Arduino.h>

#include "imu_manager.h"

// Instancia de la librería
IMUManager imu;

// Define tus pines I2C (ajusta según tu placa)
const int PIN_SDA = 21; 
const int PIN_SCL = 22;

void setup() {
    Serial.begin(115200);
    Serial.println("--- Probando IMU WT901B ---");

    // Inicializar el sensor
    if (imu.begin(PIN_SDA, PIN_SCL)) {
        Serial.println("Sensor detectado correctamente.");
    } else {
        Serial.println("Error: No se encontró el sensor en la dirección 0x50.");
        while (1); // Detener ejecución si falla
    }

    // Opcional: Resetear el ángulo al inicio
    imu.resetYaw();
    Serial.println("Yaw reseteado a 0.");
}

void loop() {
    // Actualizar datos del sensor
    imu.update();

    // Obtener valor en radianes
    float yawRad = imu.getYawRad();
    
    // Convertir a grados para que sea más fácil de leer en el monitor
    float yawDeg = yawRad * (180.0f / PI);

    // Imprimir resultados
    Serial.print("Yaw (Rad): ");
    Serial.print(yawRad, 4);
    Serial.print(" | Yaw (Deg): ");
    Serial.println(yawDeg, 2);

    // Pequeña pausa para no saturar el Serial
    delay(50); 
}