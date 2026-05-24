#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <ICM_20948.h> // Nueva librería para el ICM-20948

class IMUManager {
    private:
        ICM_20948_I2C imu;          // Objeto del nuevo sensor
        float _currentYaw = 0;      // Yaw actual en radianes
        float _yawOffset = 0;       // Offset acumulado para el "reset"

    public:
        // Constructor vacío
        IMUManager();

        // Inicializa el bus I2C y verifica la conexión con el sensor
        bool begin();

        // Lee el magnetómetro y actualiza el valor de Yaw
        void update();

        // Devuelve la orientación actual en radianes (-PI a PI)
        float getYawRad();

        // Pone la orientación actual a cero (define el "frente" del robot)
        void resetYaw();
};