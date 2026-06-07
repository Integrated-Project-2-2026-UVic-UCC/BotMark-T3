// Header file defining  the EncoderHandler class for processing quadrature encoder signals,
// calculating linear displacement, and determining wheel velocity.

#pragma once

#include <Arduino.h>
#include <ESP32Encoder.h> 

// Mechanical configuration parameters required for converting encoder ticks to metric units
struct RobotPhysics {
    float wheel_diameter; 
    int ppr;             
    float gear_ratio;     
};

// Hardware pin mapping for a single quadrature encoder
struct EncoderConfig {
    int pin_a;
    int pin_b;
};

class EncoderHandler {
public:
    // Constructor initializes the handler with pin mappings and mechanical properties
    EncoderHandler(EncoderConfig config_right, EncoderConfig config_left, RobotPhysics robot_physics);

    // Initializes the hardware counters, internal pull-ups, and noise filters
    void begin();

    // Core processing loop to update tick counts and calculate instantaneous velocities
    void update(float delta_time = 0.0);

    // Data retrieval methods for raw tick counts
    long getTicksLeft();
    long getTicksRight();

    // Data retrieval methods for instantaneous linear velocity (m/s)
    float getVelocityLeft();
    float getVelocityRight();

    // Data retrieval methods for absolute distance traveled (meters)
    float getDistLeft();
    float getDistRight();

private:
    // Hardware abstractions for the left and right pulse counters
    ESP32Encoder _encoder_right;
    ESP32Encoder _encoder_left;

    // Physical parameters and pre-calculated conversion factors
    RobotPhysics _robot_physics;
    float _meters_per_tick;

    // Pin assignments stored during instantiation for delayed initialization
    int _pin_right_a, _pin_right_b;
    int _pin_left_a, _pin_left_b;

    // State variables for tracking time and previous tick counts
    unsigned long _last_update_time;
    long _last_ticks_left;
    long _last_ticks_right;

    // Output variables storing the calculated kinematic metrics
    float _vel_left = 0;
    float _vel_right = 0;
    float _total_dist_left = 0;
    float _total_dist_right = 0;
};