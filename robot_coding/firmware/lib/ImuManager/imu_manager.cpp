// Implementation of the IMUManager library for interfacing with the ICM-20948 sensor,
// handling initialization, data acquisition, and yaw orientation calculations.

#include "imu_manager.h"

IMUManager::IMUManager() {}

bool IMUManager::begin(int sda, int scl) {    
    // Initialize the I2C bus using the specified SDA and SCL pins
    Wire.begin(sda, scl);

    // Establish communication with the sensor at the default address 0x68
    _imu_sensor.begin(Wire, 0x68);

    // Verify successful initialization of the hardware
    if (_imu_sensor.status != ICM_20948_Stat_Ok) {
        return false;
    }

    return true;
}

void IMUManager::update() {
    // Verify if new sensor data is available for processing
    if (_imu_sensor.dataReady()) {
        
        // Retrieve the latest sensor readings into the library's internal memory
        _imu_sensor.getAGMT();

        // Extract raw magnetometer values
        float mag_x = _imu_sensor.magX();
        float mag_y = _imu_sensor.magY();

        // Calculate the absolute yaw angle in radians
        // The atan2 function inherently returns a value mapped between -PI and PI
        float raw_rad = atan2(mag_y, mag_x);
        
        // Apply the calibration offset to establish the relative zero heading
        float diff = raw_rad - _yaw_offset;

        // Perform circular normalization to maintain the output strictly within the -PI to PI range
        _current_yaw = atan2(sin(diff), cos(diff));
    }
}

float IMUManager::getAccelX() { return 0.0f; }
float IMUManager::getAccelY() { return 0.0f; }
float IMUManager::getAccelZ() { return 0.0f; }

float IMUManager::getGyroX()  { return 0.0f; }
float IMUManager::getGyroY()  { return 0.0f; }
float IMUManager::getGyroZ()  { return 0.0f; }

float IMUManager::getYawRad() { return _current_yaw; }

void IMUManager::resetYaw() {
    // Capture a single immediate reading to establish the new forward reference
    _imu_sensor.getAGMT();
    _yaw_offset = atan2(_imu_sensor.magY(), _imu_sensor.magX());
}