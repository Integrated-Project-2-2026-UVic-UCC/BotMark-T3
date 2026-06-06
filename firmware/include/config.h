#pragma once

// --- Pines ---
namespace Pin {
    constexpr int M_IZQ_IN1 = 16, M_IZQ_IN2 = 17, M_IZQ_PWM = 32;
    constexpr int M_DER_IN1 = 2, M_DER_IN2 = 15, M_DER_PWM = 33;
    constexpr int ENC_IZQ_A = 12, ENC_IZQ_B = 14; 
    constexpr int ENC_DER_A = 26, ENC_DER_B = 27;
    constexpr int IMU_SDA = 21, IMU_SCL = 22;
}

// --- Física y Control ---
namespace Phys {
    constexpr float WHEEL_DIAMETER = 0.08f;
    constexpr float WHEEL_TRACK = 0.235f; // Distancia entre ruedas
    constexpr int PPR = 11;
    constexpr float GEAR_RATIO = 700.0f;
}
// --- Ajustes de PID ---
namespace PID {
    // Rueda Izquierda
    constexpr float IZQ_KP = 0.5f, IZQ_KI = 0.1f, IZQ_KD = 0.05f;
    constexpr float IZQ_MIN = -255.0f, IZQ_MAX = 255.0f;

    // Rueda Derecha
    constexpr float DER_KP = 0.5f, DER_KI = 0.1f, DER_KD = 0.05f;
    constexpr float DER_MIN = -255.0f, DER_MAX = 255.0f;
    
    // Yaw (Giro)
    constexpr float YAW_KP = 2.0f, YAW_KI = 0.0f, YAW_KD = 0.1f;
    constexpr float YAW_MIN = -0.5f, YAW_MAX = 0.5f;
}

// --- Tiempos ---
namespace Timing {
        constexpr int CONTROL_TIME_MS = 10;
        constexpr int TELEMETRY_TIME_MS = 100;
    }
