#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "MPU9250.h" // Incluimos la nueva librería

class IMUManager {
    private:
        MPU9250 mpu;                // Creamos el objeto de la MPU-9250
        float _currentYaw = 0;      // Yaw actual en Radianes
        float _yawOffset = 0;       // Offset acumulado para correcciones de pose

    public:
        // Constructor vacío
        IMUManager();

        // Inicializa el bus I2C y verifica la conexión con el sensor
        bool begin();

        // Lee los registros del sensor y actualiza el valor de Yaw
        void update();

        // Devuelve la orientación actual en radianes (-PI a PI)
        float getYawRad();

        // Pone la orientación actual a cero (define el "frente" del robot)
        void resetYaw();
};