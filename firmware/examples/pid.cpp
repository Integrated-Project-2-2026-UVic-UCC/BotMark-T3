// Calibración PID con teleplot
#include <Arduino.h>

#include "config.h"

#include "motors_controller.h"
#include "encoder_handler.h"
#include "pid_controller.h"

// 1. Hardware y Física (Asegúrate de tus valores reales)
MotorHW hwIzq = {Pin::M_IZQ_IN1, Pin::M_IZQ_IN2, Pin::M_IZQ_PWM};
MotorHW hwDer = {Pin::M_DER_IN1, Pin::M_DER_IN2, Pin::M_DER_PWM};

EncoderConfig encIzq = {Pin::ENC_IZQ_A, Pin::ENC_IZQ_B};
EncoderConfig encDer = {Pin::ENC_DER_A, Pin::ENC_DER_B};

RobotPhysics phys = {Phys::WHEEL_DIAMETER, Phys::PPR, Phys::GEAR_RATIO}; 

MotorsController robot(hwIzq, hwDer);
EncoderHandler encoders(encDer, encIzq, phys);

// 2. PID para rueda IZQUIERDA (Ajusta estos valores)
PIDController pidIzq(0.1, 0.0, 0.0, -255.0, 255.0);

unsigned long lastTime = 0;
float targetVelocity = 0.05; // 0.05 m/s es una buena velocidad de test

void setup() {
    Serial.begin(115200);
    robot.begin();
    encoders.begin();
    Serial.println("--- MODO CALIBRACION PID ---");
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    
    if (dt >= 0.02) { // 50 Hz
        encoders.update();
        float actualVelocity = encoders.getVelocityIzq();
        
        int pwm = (int)pidIzq.linealcompute(targetVelocity, actualVelocity, dt);
        robot.move(pwm, 0); // Mueve solo izquierda
        
        // Formato para Teleplot
        Serial.print(">target:"); Serial.println(targetVelocity);
        Serial.print(">actual:"); Serial.println(actualVelocity);
        Serial.print(">pwm:"); Serial.println(pwm);
        
        lastTime = now;
    }
}