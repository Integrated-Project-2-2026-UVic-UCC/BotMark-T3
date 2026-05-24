#include "imu_manager.h"

IMUManager::IMUManager() {}

bool IMUManager::begin() {    
    // 1. Iniciar bus I2C con los pines de la ESP32 (SDA=21, SCL=22)
    Wire.begin(21, 22);

    // 2. Iniciar la comunicación con el sensor en la dirección 0x68
    imu.begin(Wire, 0x68);

    // 3. Comprobar si se ha inicializado correctamente
    if (imu.status != ICM_20948_Stat_Ok) {
        return false;
    }

    return true;
}

void IMUManager::update() {
    // Comprobar si el sensor tiene datos frescos listos para leer
    if (imu.dataReady()) {
        
        // Cargar las lecturas en la memoria de la librería
        imu.getAGMT();

        // Extraer datos crudos del magnetómetro
        float mx = imu.magX();
        float my = imu.magY();

        // Calcular el Yaw en radianes directamente.
        // La función atan2(y, x) de C++ ya devuelve matemáticamente 
        // un valor entre -PI y PI, que es exactamente lo que ROS necesita.
        float rawRad = atan2(my, mx);
        
        // Aplicar el desfase para poder "poner a cero" el frente del robot
        float diff = rawRad - _yawOffset;

        // Normalización circular esencial:
        // Garantiza que la salida final siga respetando el límite de -PI a PI
        // aunque el robot dé varias vueltas sobre sí mismo.
        _currentYaw = atan2(sin(diff), cos(diff));
    }
}

float IMUManager::getYawRad() { 
    return _currentYaw; 
}

void IMUManager::resetYaw() {
    // Tomamos una lectura instantánea para fijarla como el nuevo "cero"
    imu.getAGMT();
    _yawOffset = atan2(imu.magY(), imu.magX());
}