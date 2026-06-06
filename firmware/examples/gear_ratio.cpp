#include <Arduino.h>
#include "motors_controller.h"
#include "encoder_handler.h"

// Configuración inicial (usa tu valor teórico de 200.0)
RobotPhysics phys = {0.08, 11, 700.0}; 
MotorHW hwIzq = {16, 17, 32};
MotorHW hwDer = {2, 15, 33};
EncoderConfig encIzq = {12, 14};
EncoderConfig encDer = {26, 27};

MotorsController robot(hwIzq, hwDer);
EncoderHandler encoders(encDer, encIzq, phys);

void setup() {
    Serial.begin(115200);
    robot.begin();
    encoders.begin();
    
    delay(3000);

    // Mueve el robot a una velocidad baja constante
    robot.move(255, 255); 
    delay(2000);
    robot.move(0, 0);

    encoders.update();
    
    // Imprime los resultados para que los veas en el Serial Monitor
    Serial.print("Ticks Izq: "); Serial.println(encoders.getTicksIzq());
    Serial.print("Ticks Der: "); Serial.println(encoders.getTicksDer());
}

void loop() {
    // Nada que hacer aquí
}