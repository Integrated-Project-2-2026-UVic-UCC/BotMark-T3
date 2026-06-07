// Header file defining the PIDController library for calculating proportional, integral, 
// and derivative responses for linear displacement and angular rotation.

#pragma once

#include <Arduino.h>

class PIDController {
public:
    PIDController(float kp, float ki, float kd, float min_out, float max_out, float initial_angle = 0.0);
    
    float linealCompute(float setpoint, float current_value, float delta_time);
    float circularCompute(float setpoint, float current_value, float delta_time);

    void reset(float current_angle = 0.0);

private:
    float _kp, _ki, _kd;
    float _min_out, _max_out, _target_angle;
    float _accumulated_error;
    float _last_error;
    unsigned long _last_time;
};