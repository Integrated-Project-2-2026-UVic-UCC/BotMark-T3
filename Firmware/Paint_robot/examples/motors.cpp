#include <Arduino.h>

#include "motors_controller.h"

// Define los pines según tu hardware
// MotorHW {IN1, IN2, PWM}
MotorHW motorIzquierdo = {16, 17, 32}; 
MotorHW motorDerecho   = {2, 15, 33};

MotorsController robot(motorIzquierdo, motorDerecho);

void setup() {
    Serial.begin(115200);
    robot.begin();
    Serial.println("--- Test de Motores Iniciado ---");
}

void loop() {
    // TEST 1: Avance progresivo (Rampa de velocidad)
    Serial.println("Test 1: Avance progresivo");
    for (int i = 0; i <= 255; i += 50) {
        robot.move(i, i);
        delay(500);
    }
    
    // TEST 2: Frenado y Reversa
    Serial.println("Test 2: Reversa total");
    robot.move(-200, -200);
    delay(2000);

    // TEST 3: Giro sobre el eje (Tanque)
    Serial.println("Test 3: Giro derecha");
    robot.move(150, -150);
    delay(1000);

    // TEST 4: Verificación del Escalado Inteligente
    // Enviamos valores superiores a 255. 
    // La librería debería escalar (400, 200) -> (255, 127) aprox.
    Serial.println("Test 4: Probando escalado inteligente (>255)");
    robot.move(500, 250); 
    delay(2000);

    // Parada total
    Serial.println("Parada...");
    robot.move(0, 0);
    delay(3000);
}