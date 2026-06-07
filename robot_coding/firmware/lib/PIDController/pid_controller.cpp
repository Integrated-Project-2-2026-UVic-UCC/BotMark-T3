// Implementation of the PIDController library for calculating proportional, integral, 
// and derivative responses for linear displacement and angular rotation.

#include "pid_controller.h"

PIDController::PIDController(float kp, float ki, float kd, float min_out, float max_out, float initial_angle) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _min_out = min_out;
    _max_out = max_out;
    _target_angle = initial_angle;
    _accumulated_error = 0.0;
    _last_error = 0.0;
}

float PIDController::linealCompute(float setpoint, float current_value, float delta_time) {
    // Prevent division by zero if the control loop executes too rapidly
    if (delta_time <= 0.0) return 0.0; 

    float error = setpoint - current_value;

    // Calculate proportional term
    float p = _kp * error;

    // Calculate integral term and apply dynamic anti-windup limits
    _accumulated_error += error * delta_time;
    _accumulated_error = constrain(_accumulated_error, -50.0, 50.0); 
    float i = _ki * _accumulated_error;

    // Calculate derivative term based on the rate of error change
    float d = _kd * ((error - _last_error) / delta_time);

    // Store current error for the next derivative calculation
    _last_error = error;

    // Aggregate terms and constrain the final output to the permissible PWM range
    float output = p + i + d;
    return constrain(output, _min_out, _max_out);
}

float PIDController::circularCompute(float target_rate, float current_value, float delta_time) {
    // Prevent division by zero if the control loop executes too rapidly
    if (delta_time <= 0.0) return 0.0; 

    // Advance the target angle based on the specified rotation rate
    _target_angle += target_rate * delta_time;
    _target_angle = atan2(sin(_target_angle), cos(_target_angle));

    // Calculate the shortest path angular error to prevent erratic behavior at PI bounds
    float error = atan2(sin(_target_angle - current_value), cos(_target_angle - current_value));

    // Calculate proportional term
    float p = _kp * error;

    // Calculate integral term and apply dynamic anti-windup limits for angular bounds
    _accumulated_error += error * delta_time;
    _accumulated_error = constrain(_accumulated_error, -2.0, 2.0); 
    float i = _ki * _accumulated_error;

    // Calculate derivative term based on the rate of error change
    float d = _kd * ((error - _last_error) / delta_time);

    // Store current error for the next derivative calculation
    _last_error = error;

    // Aggregate terms and constrain the final output to the permissible PWM range
    float output = p + i + d;
    return constrain(output, _min_out, _max_out);
}

void PIDController::reset(float current_angle) {
    // Clear accumulated states to prevent sudden actuation jumps upon resuming movement
    _accumulated_error = 0.0;
    _last_error = 0.0;
    _target_angle = current_angle;
}