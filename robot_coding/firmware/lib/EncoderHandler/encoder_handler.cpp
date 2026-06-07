// Implementation of the EncoderHandler class for processing quadrature encoder signals,
// calculating linear displacement, and determining wheel velocity.

#include "encoder_handler.h"

EncoderHandler::EncoderHandler(EncoderConfig config_right, EncoderConfig config_left, RobotPhysics robot_physics) : _robot_physics(robot_physics) {
    _pin_right_a = config_right.pin_a; 
    _pin_right_b = config_right.pin_b;
    _pin_left_a = config_left.pin_a; 
    _pin_left_b = config_left.pin_b;

    // Calculate the linear distance traveled per individual encoder pulse in meters
    _meters_per_tick = (PI * robot_physics.wheel_diameter) / (robot_physics.ppr * robot_physics.gear_ratio * 4.0);
}

void EncoderHandler::begin() {
    // Enable internal pull-up resistors to maintain a known high state when no signal is actively driving the pin
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    // Configure pins for full quadrature mode, evaluating both rising and falling edges on both channels for 4x resolution
    _encoder_left.attachFullQuad(_pin_left_a, _pin_left_b);
    _encoder_right.attachFullQuad(_pin_right_a, _pin_right_b);
    
    // Apply a hardware filter to ignore transient noise pulses shorter than 100 clock cycles
    _encoder_left.setFilter(100); 
    _encoder_right.setFilter(100);

    // Initialize baseline tick counts to prevent velocity calculation spikes during the first update cycle
    _last_ticks_left = _encoder_left.getCount();
    _last_ticks_right = _encoder_right.getCount();
}

void EncoderHandler::update(float delta_time) {

    // Ensure a minimum time delta has passed to prevent division by zero and calculation instability
    if (delta_time >= 0.01) { 

        // Retrieve absolute tick counts from the hardware timers
        long current_ticks_left = _encoder_left.getCount();
        long current_ticks_right = _encoder_right.getCount();

        // Calculate the discrete change in ticks since the last execution cycle
        long left_ticks_increased = current_ticks_left - _last_ticks_left;
        long right_ticks_increased = current_ticks_right - _last_ticks_right;

        // Compute linear displacement based on the tick variation
        float left_dist_increased = left_ticks_increased * _meters_per_tick;
        float right_dist_increased = right_ticks_increased * _meters_per_tick;

        // Calculate instantaneous velocity in meters per second
        _vel_left = left_dist_increased / delta_time;
        _vel_right = right_dist_increased / delta_time;

        // Aggregate total distance traveled (retained as comments for debugging purposes)
        // _total_dist_left += left_dist_increased;
        // _total_dist_right += right_dist_increased;

        // Store current tick counts for the next control iteration
        _last_ticks_left = current_ticks_left;
        _last_ticks_right = current_ticks_right;
    }
}

// Data retrieval methods for raw tick counts
long EncoderHandler::getTicksLeft() { return _last_ticks_left; }
long EncoderHandler::getTicksRight() { return _last_ticks_right; }

// Data retrieval methods for instantaneous linear velocity (m/s)
float EncoderHandler::getVelocityLeft() { return _vel_left; }
float EncoderHandler::getVelocityRight() { return _vel_right; }

// Data retrieval methods for absolute distance traveled (meters)
float EncoderHandler::getDistLeft() { return _total_dist_left; }
float EncoderHandler::getDistRight() { return _total_dist_right; }