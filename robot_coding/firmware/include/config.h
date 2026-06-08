// This header file defines the global configuration parameters for the robot,
// including hardware pin assignments, physical dimensions, PID controller 
// constants, and system timing intervals.

#pragma once

// Hardware pin assignments
namespace Pin {
    constexpr int M_LEFT_IN1 = 16, M_LEFT_IN2 = 17, M_LEFT_PWM = 32;
    constexpr int M_RIGHT_IN1 = 2, M_RIGHT_IN2 = 15, M_RIGHT_PWM = 33;
    constexpr int ENC_LEFT_A = 12, ENC_LEFT_B = 14; 
    constexpr int ENC_RIGHT_A = 26, ENC_RIGHT_B = 27;
    constexpr int IMU_SDA = 21, IMU_SCL = 22;
    constexpr int SERVO = 25;
}

// Mechanical properties and physical measurements
namespace Phys {
    constexpr float WHEEL_DIAMETER = 0.08f;
    constexpr float WHEEL_TRACK = 0.235f; 
    constexpr int PPR = 11;
    constexpr float GEAR_RATIO = 700.0f;
}

// Proportional-Integral-Derivative (PID) controller tuning parameters
namespace PID {
    // Left wheel PID constants and output limits
    constexpr float LEFT_KP = 0.5f, LEFT_KI = 0.1f, LEFT_KD = 0.05f;
    constexpr float LEFT_MIN = -255.0f, LEFT_MAX = 255.0f;

    // Right wheel PID constants and output limits
    constexpr float RIGHT_KP = 0.5f, RIGHT_KI = 0.1f, RIGHT_KD = 0.05f;
    constexpr float RIGHT_MIN = -255.0f, RIGHT_MAX = 255.0f;
    
    // Yaw orientation PID constants and output limits
    constexpr float YAW_KP = 2.0f, YAW_KI = 0.0f, YAW_KD = 0.1f;
    constexpr float YAW_MIN = -0.5f, YAW_MAX = 0.5f;
}

// Loop execution and telemetry transmission intervals
namespace Timing {
    constexpr int CONTROL_TIME_MS = 10;
    constexpr int TELEMETRY_TIME_MS = 100;
}

namespace ServoAngles {
    constexpr float CLOSE_ANGLE = 0.0f;
    constexpr float OPEN_ANGLE = 180.0f;
}