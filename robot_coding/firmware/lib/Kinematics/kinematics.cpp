// Implementation of the Kinematics library for calculating inverse kinematics 
// on a differential drive system.

#include "kinematics.h"

Kinematics::Kinematics(float track_width) : _track_width(track_width) {}

WheelSpeeds Kinematics::calculateWheelSpeeds(float linear_velocity, float angular_velocity) {
    WheelSpeeds wheel_speeds;
    
    // Calculate required speed for the left wheel based on linear and angular components
    wheel_speeds.left = linear_velocity - (angular_velocity * _track_width / 2.0f);
    
    // Calculate required speed for the right wheel based on linear and angular components
    wheel_speeds.right = linear_velocity + (angular_velocity * _track_width / 2.0f);
    
    return wheel_speeds;
}