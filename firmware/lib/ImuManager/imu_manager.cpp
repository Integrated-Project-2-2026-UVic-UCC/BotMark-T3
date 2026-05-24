#include "imu_manager.h"

IMUManager::IMUManager() {}

bool IMUManager::begin() {    
    // Inicia la IMU en la dirección 0x68 (asegúrate de que el pin AD0 está conectado a GND)
    if (!mpu.setup(0x69)) {
        return false;
    }

    // AVISO DE CALIBRACIÓN
    // Para que el Yaw sea estable, el magnetómetro debe calibrarse al encender.
    // Descomenta la siguiente línea y la IMU pausará el código unos segundos
    // para que la muevas en forma de "8" y se calibre correctamente.
    
    // mpu.calibrateMag();

    // Filtro Mahony: es más ligero para la CPU de la ESP32 que el Madgwick
    // y para un robot móvil suele dar resultados muy estables.
    mpu.selectFilter(QuatFilterSel::MAHONY);

    return true;
}

void IMUManager::update() {
    // mpu.update() lee todos los sensores, alimenta el filtro interno
    // y devuelve 'true' si hay datos nuevos.
    if (mpu.update()) {
        
        // La librería de hideakitai devuelve el Yaw directamente en grados
        float deg = mpu.getYaw();
        
        // Pasamos a radianes
        float rawRad = deg * (PI / 180.0f);
        
        float diff = rawRad - _yawOffset;

        // Normalización esencial: mantiene el ángulo entre -PI y PI
        // Fundamental para que el PID de ROS no intente dar una vuelta de 360°
        _currentYaw = atan2(sin(diff), cos(diff));
    }
}

float IMUManager::getYawRad() { 
    return _currentYaw; 
}

void IMUManager::resetYaw() {
    _yawOffset = mpu.getYaw() * (PI / 180.0f);
}