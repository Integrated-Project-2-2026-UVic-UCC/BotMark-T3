#include <Arduino.h>

#include <config.h>
#include <secrets.h>

// Librerías de tu robot
#include "motors_controller.h"
#include "encoder_handler.h"
#include "imu_manager.h"
#include "kinematics.h"
#include "pid_controller.h"
#include "web_app_controller.h"
#include "zenoh_manager.h"

// 1. Configuración de Hardware
MotorHW hwIzq = {Pin::M_IZQ_IN1, Pin::M_IZQ_IN2, Pin::M_IZQ_PWM};
MotorHW hwDer = {Pin::M_DER_IN1, Pin::M_DER_IN2, Pin::M_DER_PWM};

EncoderConfig encIzq = {Pin::ENC_IZQ_A, Pin::ENC_IZQ_B};
EncoderConfig encDer = {Pin::ENC_DER_A, Pin::ENC_DER_B};

RobotPhysics phys = {Phys::WHEEL_DIAMETER, Phys::PPR, Phys::GEAR_RATIO}; 

// 2. Instancias
MotorsController robot(hwIzq, hwDer);
EncoderHandler encoders(encDer, encIzq, phys); 
IMUManager imu;
Kinematics kinematics(Phys::WHEEL_TRACK); // Distancia entre ruedas
PIDController pidYaw(PID::YAW_KP, PID::YAW_KI, PID::YAW_KD, PID::YAW_MIN, PID::YAW_MAX);
PIDController pidIzq(PID::IZQ_KP, PID::IZQ_KI, PID::IZQ_KD, PID::IZQ_MIN, PID::IZQ_MAX);
PIDController pidDer(PID::DER_KP, PID::DER_KI, PID::DER_KD, PID::DER_MIN, PID::DER_MAX);
WebAppController webServer(80);
ZenohManager zm;

unsigned long lastControlTime = 0;
unsigned long lastZenohTime = 0;
bool is_stopped = true; // Por seguridad, arrancamos bloqueados
bool is_paused = false;

// --- 5. Funciones Callback para la Web App ---
void handleStop() {
    is_stopped = true;
    zm.publishStopped(true);
    Serial.println("Estado: STOP");
}

void handleStart() {
    is_stopped = false;
    zm.publishStopped(false);
    // Reiniciamos PID al arrancar para evitar saltos bruscos
    pidIzq.reset();
    pidDer.reset();
    Serial.println("Estado: START");
}

void handlePause() {
    is_paused = true;
    zm.publishPaused(true);
    Serial.println("Estado: PAUSE");
}

void handleResume() {
    is_paused = false;
    zm.publishPaused(false);
    Serial.println("Estado: RESUME");
}

void setup (){
Serial.begin(115200);
    
    // Inicialización física
    robot.begin();
    encoders.begin();

    // Inicialización IMU con reintentos
    Serial.println("Conectando IMU...");
    while (!imu.begin(Pin::IMU_SDA, Pin::IMU_SCL)) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nIMU iniciada correctamente!");
    Serial.println("Mantén el robot quieto para fijar el cero inicial...");
    delay(2000); // Pequeña pausa para estabilizar lecturas
    imu.resetYaw();
    Serial.println("Yaw reseteado a 0");

    // Vincular funciones a la Web App y arrancar servidor
    webServer.onStop(handleStop);
    webServer.onStart(handleStart);
    webServer.onPause(handlePause);
    webServer.onResume(handleResume);
    webServer.begin(Network::SSID, Network::PASSWORD);

    // Inicialización Zenoh con reintentos
    // (Asegúrate de que los parámetros coincidan con la firma de tu librería)
    Serial.println("Conectando Zenoh...");
    while (!zm.begin(Network::ROUTER_IP)) { 
        Serial.print(".");
        delay(300);
    }
    Serial.println("\n¡Zenoh Manager iniciado correctamente!");
}

// --- 7. Bucle Principal (No bloqueante) ---
void loop() {
    // Escucha permanente de comandos Web
    webServer.handleClient();
    
    unsigned long now = millis();

    if (now - lastControlTime >= Timing::CONTROL_TIME_MS) {
        
        encoders.update();
        imu.update();

        float current_yaw = imu.getYawRad();

        if (is_stopped || is_paused || zm.isObstacleDetected()) {
            robot.move(0, 0);
            pidIzq.reset();
            pidDer.reset();
            pidYaw.reset(current_yaw);
        } else {
            float dt = (now - lastControlTime) / 1000.0;

            Twist cmd = zm.getLastCommand();
            
            float corrected_angular_z = cmd.angular_z + pidYaw.circularcompute(cmd.angular_z, current_yaw, dt);

            WheelSpeeds target = kinematics.calculateWheelSpeeds(cmd.linear_x, corrected_angular_z);

            // Calcular PID de las ruedas
            int outIzq = (int)pidIzq.linealcompute(target.left, encoders.getVelocityIzq(), dt);
            int outDer = (int)pidDer.linealcompute(target.right, encoders.getVelocityDer(), dt);
            robot.move(outIzq, outDer);
        }

        lastControlTime = now;
    }

    if (now - lastZenohTime >= Timing::TELEMETRY_TIME_MS) {
        if (zm.isConnected()) {
            SensorData data;
            
            // 1. Lectura real de los Encoders
            data.ticks_izquierdo = encoders.getTicksIzq(); 
            data.ticks_derecho   = encoders.getTicksDer();   
            
            // 2. Lectura real de la IMU (Acelerómetro y Giroscopio)
            data.accel_x = imu.getAccelX();         
            data.accel_y = imu.getAccelY();
            data.accel_z = imu.getAccelZ();
            data.gyro_x  = imu.getGyroX();
            data.gyro_y  = imu.getGyroY();
            data.gyro_z  = imu.getGyroZ();
            
            zm.publishSensors(data);
        }

        lastZenohTime = now;
    }
}