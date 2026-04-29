#include <Arduino.h>

#include "config.h"           // Donde tienes tus pines y constantes
#include "motors_controller.h"
#include "encoder_handler.h"
#include "pid_controller.h"

// 1. Instancias de hardware (Ajusta los nombres según tus constructores)
MotorsController motores(PIN_MOTOR_IZQ_IN1, PIN_MOTOR_IZQ_IN2, PIN_MOTOR_IZQ_PWM,
                        PIN_MOTOR_DER_IN1, PIN_MOTOR_DER_IN2, PIN_MOTOR_DER_PWM);

// Ajusta Diámetro, PPR y Ratio según tu robot
RobotPhysics fisica = {0.065, 11, 30.0}; 
EncoderHandler encoders(PIN_ENC_IZQ_A, PIN_ENC_IZQ_B, PIN_ENC_DER_A, PIN_ENC_DER_B, fisica);

// 2. Instancias del PID
// Empezamos con valores bajos para no quemar nada: Kp=1.0, Ki=0.0, Kd=0.0
// Rango de salida: -255 a 255 (PWM)
PidController pidIzq(1.0, 0.0, 0.0, -255.0, 255.0);
PidController pidDer(1.0, 0.0, 0.0, -255.0, 255.0);

float setpointGlobal = 0.0; // Velocidad deseada en m/s

void setup() {
    Serial.begin(115200);
    motores.begin();
    encoders.begin();
    
    Serial.println("--- Iniciando Test PID ---");
    delay(2000); // Tiempo para dejar el robot en el suelo
}

void loop() {
    // --- LÓGICA DE PRUEBA (CAMBIO DE VELOCIDAD CADA 5 SEG) ---
    if (millis() % 10000 < 5000) {
        setpointGlobal = 0.3; // Ir a 0.3 m/s
    } else {
        setpointGlobal = 0.0; // Parar
    }

    // 1. Actualizar Encoders
    encoders.update();
    float velRealIzq = encoders.getVelocityIzq();
    float velRealDer = encoders.getVelocityDer();

    // 2. Calcular PID para cada rueda
    float pwmIzq = pidIzq.compute(setpointGlobal, velRealIzq);
    float pwmDer = pidDer.compute(setpointGlobal, velRealDer);

    // 3. Aplicar potencia a los motores
    motores.move(pwmIzq, pwmDer);

    // 4. TELEMETRÍA PARA TELEPLOT
    // Solo graficamos la izquierda para tunear primero un lado
    Serial.print(">Target:");
    Serial.println(setpointGlobal);
    
    Serial.print(">Real_Izq:");
    Serial.println(velRealIzq);

    Serial.print(">PWM_Escalado:");
    Serial.println(pwmIzq / 100.0); // PWM de 0 a 2.55 para que quepa en la gráfica

    delay(10); // Frecuencia de control de 100Hz aprox
}