#pragma once

// --- PINES MOTORES (Basados en tus pruebas) ---
// Motor Izquierdo
#define PIN_MOTOR_IZQ_IN1 16
#define PIN_MOTOR_IZQ_IN2 17
#define PIN_MOTOR_IZQ_PWM 32

// Motor Derecho
#define PIN_MOTOR_DER_IN1 2
#define PIN_MOTOR_DER_IN2 15
#define PIN_MOTOR_DER_PWM 33

// --- PINES ENCODERS ---
// Rueda Izquierda
#define PIN_ENC_IZQ_A 12
#define PIN_ENC_IZQ_B 14

// Rueda Derecha
#define PIN_ENC_DER_A 26
#define PIN_ENC_DER_B 27

// --- PARÁMETROS FÍSICOS DEL ROBOT ---
// Diámetro de la rueda en metros (ej: 65mm = 0.065)
#define ROBOT_WHEEL_DIAMETER 0.065

// Pulsos por revolución del motor (antes de la reductora)
#define ROBOT_ENCODER_PPR 11

// Relación de la reductora (ej: 30:1)
#define ROBOT_GEAR_RATIO 30.0

// Ancho de vía efectivo (distancia entre ruedas en metros)
// Como es Skid-Steer, empieza con la medida real y ajústala luego
#define ROBOT_TRACK_WIDTH 0.30 

// --- CONFIGURACIÓN DE CONTROL ---
#define LOOP_PERIOD_MS 10  // Frecuencia de control (100Hz)