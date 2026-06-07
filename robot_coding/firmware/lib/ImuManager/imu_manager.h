// Header file defining the IMUManager library for interfacing with the ICM-20948 sensor,
// handling initialization, data acquisition, and yaw orientation calculations.

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <ICM_20948.h> 

class IMUManager {
public:
    IMUManager();

    bool begin(int sda, int scl);

    void update();

    float getAccelX();
    float getAccelY();
    float getAccelZ();
    
    float getGyroX();
    float getGyroY();
    float getGyroZ();

    float getYawRad();

    void resetYaw();
private:
    ICM_20948_I2C _imu_sensor;          
    float _current_yaw = 0;      
    float _yaw_offset = 0;       
};