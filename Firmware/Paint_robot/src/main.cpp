#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("A");
}

void loop() {
    Serial.print("B");
    delay(2000); 
}