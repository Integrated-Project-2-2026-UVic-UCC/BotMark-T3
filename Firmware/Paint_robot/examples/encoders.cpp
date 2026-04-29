#include <Arduino.h>

#include "motors_controller.h"
#include "encoder_handler.h"

// 1. Configuración de Hardware
MotorHW hwIzq = {16, 17, 32}; // IN1, IN2, PWM
MotorHW hwDer = {2, 15, 33};

EncoderConfig encIzq = {12, 14}; // Pin A, Pin B
EncoderConfig encDer = {26, 27};

// 2. Parámetros Físicos (Ajusta estos valores a tu robot)
// Diámetro: 0.065m (65mm), PPR: 11, Ratio: 30:1
RobotPhysics phys = {0.065, 11, 30.0}; 

// 3. Instancias
MotorsController robot(hwIzq, hwDer);
EncoderHandler encoders(encDer, encIzq, phys); // Ojo: verifica orden Der/Izq según tu constructor

void setup() {
    Serial.begin(115200);
    
    robot.begin();
    encoders.begin();

    Serial.println("--- Test de Distancia: Objetivo 0.5 metros ---");
    delay(2000); // Tiempo para soltar el robot en el suelo
}

void loop() {
    // Actualizamos los cálculos de los encoders
    encoders.update();

    float distanciaActual = encoders.getDistIzq(); // Usamos la rueda izquierda de referencia
    float velocidadActual = encoders.getVelocityIzq();

    Serial.print("Dist: "); Serial.print(distanciaActual);
    Serial.print(" m | Vel: "); Serial.print(velocidadActual);
    Serial.println(" m/s");

    if (distanciaActual < 0.50) {
        // Si no ha llegado a medio metro, sigue moviéndose al 50% de potencia (aprox 128)
        robot.move(128, 128);
    } else {
        // ¡Llegamos! Frenazo.
        robot.move(0, 0);
        Serial.println("--- Objetivo Alcanzado ---");
        while(1); // Bloqueamos aquí para que no haga nada más
    }

    delay(10); // Pequeña pausa para no saturar el Serial
}