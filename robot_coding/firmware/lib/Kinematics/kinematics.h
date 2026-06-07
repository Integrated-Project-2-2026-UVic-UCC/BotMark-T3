// Header file defining the Kinematics library for calculating inverse kinematics 
// on a differential drive system.

#pragma once

#include <Arduino.h>

struct WheelSpeeds {
    float left;  // Target velocity for the left wheel in m/s
    float right; // Target velocity for the right wheel in m/s
};

class Kinematics {
public:
    // Constructor initializes the kinematics model with the physical track width
    Kinematics(float track_width);

    // Inverse Kinematics: Converts standard twist velocities into required wheel speeds
    WheelSpeeds calculateWheelSpeeds(float linear_velocity, float angular_velocity);

private:
    float _track_width; // Distance between the left and right wheels in meters
};