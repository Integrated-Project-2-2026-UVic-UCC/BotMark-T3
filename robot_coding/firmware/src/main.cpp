// This program implements the main control loop for an autonomous robot, integrating 
// hardware interfaces, kinematic calculations, PID control, and network communications 
// via a web application and the Zenoh protocol.

#include <Arduino.h>

#include <motors_controller.h>
#include <encoder_handler.h>
#include <imu_manager.h>
#include <kinematics.h>
#include <pid_controller.h>
#include <web_app_controller.h>
#include <zenoh_manager.h>

#include "config.h"
#include "secrets.h"

// Hardware configuration mapping
MotorHW hw_left = {Pin::M_LEFT_IN1, Pin::M_LEFT_IN2, Pin::M_LEFT_PWM};
MotorHW hw_right = {Pin::M_RIGHT_IN1, Pin::M_RIGHT_IN2, Pin::M_RIGHT_PWM};

EncoderConfig enc_left = {Pin::ENC_LEFT_A, Pin::ENC_LEFT_B};
EncoderConfig enc_right = {Pin::ENC_RIGHT_A, Pin::ENC_RIGHT_B};

RobotPhysics robot_physics = {Phys::WHEEL_DIAMETER, Phys::PPR, Phys::GEAR_RATIO}; 

// Subsystem instances
MotorsController motorsController(hw_left, hw_right);
EncoderHandler encoderHandler(enc_right, enc_left, robot_physics); 
IMUManager imuManager;
Kinematics kinematicsManager(Phys::WHEEL_TRACK); 
PIDController pidYaw(PID::YAW_KP, PID::YAW_KI, PID::YAW_KD, PID::YAW_MIN, PID::YAW_MAX);
PIDController pidLeft(PID::LEFT_KP, PID::LEFT_KI, PID::LEFT_KD, PID::LEFT_MIN, PID::LEFT_MAX);
PIDController pidRight(PID::RIGHT_KP, PID::RIGHT_KI, PID::RIGHT_KD, PID::RIGHT_MIN, PID::RIGHT_MAX);
WebAppController webAppServer(80);
ZenohManager zenohManager;

// State and timing variables
unsigned long last_control_time = 0;
unsigned long last_zenoh_time = 0;
bool is_stopped = true; 
bool is_paused = false;

// Web application callback functions
void HandleStop() {
    is_stopped = true;
    zenohManager.publishStopped(true);
    Serial.println("State: STOP");
}

void HandleStart() {
    is_stopped = false;
    zenohManager.publishStopped(false);
    
    // Reset PID controllers on startup to prevent sudden jumps
    pidLeft.reset();
    pidRight.reset();
    Serial.println("State: START");
}

void HandlePause() {
    is_paused = true;
    zenohManager.publishPaused(true);
    Serial.println("State: PAUSE");
}

void HandleResume() {
    is_paused = false;
    zenohManager.publishPaused(false);
    Serial.println("State: RESUME");
}

void setup() {
    Serial.begin(115200);
    
    // Initialize physical hardware subsystems
    motorsController.begin();
    encoderHandler.begin();

    // Initialize IMU with connection retries
    Serial.println("Connecting IMU...");
    while (!imuManager.begin(Pin::IMU_SDA, Pin::IMU_SCL)) {
        Serial.print(".");
        delay(500);
    }
    
    Serial.println("\nIMU initialized successfully");
    Serial.println("Keep the robot steady to calibrate zero heading...");
    delay(2000); 
    imuManager.resetYaw();
    Serial.println("Yaw reset to 0");

    // Bind callback functions to the web server and initialize network
    webAppServer.onStop(HandleStop);
    webAppServer.onStart(HandleStart);
    webAppServer.onPause(HandlePause);
    webAppServer.onResume(HandleResume);
    webAppServer.begin(Network::SSID, Network::PASSWORD);

    // Initialize Zenoh communication manager with connection retries
    Serial.println("Connecting Zenoh...");
    while (!zenohManager.begin(Network::ROUTER_IP)) { 
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nZenoh Manager initialized successfully");
}

void loop() {
    // Continuously listen for incoming web application commands
    webAppServer.handleClient();
    
    unsigned long current_time = millis();

    // Execute core control loop based on defined timing interval
    if (current_time - last_control_time >= Timing::CONTROL_TIME_MS) {
        float delta_time = (current_time - last_control_time) / 1000.0;
        encoderHandler.update(delta_time);
        imuManager.update();

        float current_yaw = imuManager.getYawRad();

        // Safety check to halt movement if stopped, paused, or blocked
        if (is_stopped || is_paused || zenohManager.isObstacleDetected()) {
            motorsController.move(0, 0);
            pidLeft.reset();
            pidRight.reset();
            pidYaw.reset(current_yaw);
        } else {

            Twist command = zenohManager.getLastCommand();
            
            // Calculate yaw correction and final wheel speeds
            float corrected_angular_z = command.angular_z + pidYaw.circularCompute(command.angular_z, current_yaw, delta_time);
            WheelSpeeds target_speeds = kinematicsManager.calculateWheelSpeeds(command.linear_x, corrected_angular_z);

            // Compute PWM outputs via individual wheel PID controllers
            int out_left = (int)pidLeft.linealCompute(target_speeds.left, encoderHandler.getVelocityLeft(), delta_time);
            int out_right = (int)pidRight.linealCompute(target_speeds.right, encoderHandler.getVelocityRight(), delta_time);
            
            motorsController.move(out_left, out_right);
        }

        last_control_time = current_time;
    }

    // Transmit telemetry data based on defined timing interval
    if (current_time - last_zenoh_time >= Timing::TELEMETRY_TIME_MS) {
        if (zenohManager.isConnected()) {
            SensorData sensor_data;
            
            // Capture current encoder counts
            sensor_data.ticks_left = encoderHandler.getTicksLeft(); 
            sensor_data.ticks_right   = encoderHandler.getTicksRight();   
            
            // Capture current IMU readings
            sensor_data.accel_x = imuManager.getAccelX();         
            sensor_data.accel_y = imuManager.getAccelY();
            sensor_data.accel_z = imuManager.getAccelZ();
            sensor_data.gyro_x  = imuManager.getGyroX();
            sensor_data.gyro_y  = imuManager.getGyroY();
            sensor_data.gyro_z  = imuManager.getGyroZ();
            
            zenohManager.publishSensors(sensor_data);
        }

        last_zenoh_time = current_time;
    }
}