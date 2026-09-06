/**
 * @file motor_control.h
 * @brief Motor control interface for ClawDog quadruped robot
 * 
 * Provides motor initialization, position reading, velocity command handling,
 * and safety limit enforcement for the quadruped leg joints.
 * 
 * Hardware: ESP32-S3 with SA8301 H-bridge motor drivers
 * Control: PWM + DIR pins, position feedback via ADC (potentiometers)
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// ======================== CONFIGURATION ========================

// Motor pins (GPIO)
struct MotorPins {
    uint8_t pwm;
    uint8_t dir;
    uint8_t adc;
};

// Default pin mapping for ClawDog
const MotorPins MOTOR_PINS[4] = {
    {1, 2, 13},   // Motor 0
    {4, 5, 6},    // Motor 1
    {7, 8, 9},    // Motor 2
    {10, 11, 12}  // Motor 3
};

// PWM configuration
const int PWM_FREQ = 1000;
const int PWM_RESOLUTION = 8;
const int PWM_MAX_DUTY = 255;

// Safety limits
const float MAX_PWM_PERCENT = 0.30;  // 30% max PWM for safety
const float BATTERY_MIN_VOLTAGE = 6.0; // 2S LiPo minimum

// ADC configuration
const int ADC_RESOLUTION = 12;
const int ADC_MAX_VALUE = 4095;
const float ADC_VOLTAGE_REF = 3.3;

// Battery monitoring
const float BATTERY_ADC_DIVIDER = 2.0;  // Voltage divider ratio

// ======================== CLASS ========================

/**
 * @brief Motor control class for quadruped leg joints
 * 
 * Manages 4 DC motors with H-bridge drivers and potentiometer feedback.
 * Provides position control, velocity commands, and safety enforcement.
 */
class MotorControl {
public:
    MotorControl();
    
    /**
     * @brief Initialize motor hardware
     * 
     * Sets up PWM channels, GPIO pins, and ADC.
     * Must be called before any other motor functions.
     */
    void init();
    
    /**
     * @brief Update motor control loop
     * 
     * Should be called at regular intervals (e.g., 50ms).
     * Reads positions, updates PID, applies safety limits.
     */
    void update();
    
    /**
     * @brief Set velocity command from teleop
     * 
     * @param linear_x Forward/backward velocity
     * @param angular_z Rotation velocity
     */
    void set_velocity_command(float linear_x, float angular_z);
    
    /**
     * @brief Get current joint positions
     * 
     * @param positions Array of 4 floats to fill with joint angles (radians)
     */
    void get_joint_positions(float positions[4]);
    
    /**
     * @brief Get battery voltage
     * 
     * @return float Battery voltage in volts
     */
    float get_battery_voltage();
    
    /**
     * @brief Emergency stop all motors
     */
    void emergency_stop();
    
    /**
     * @brief Resume from emergency stop
     */
    void resume();

private:
    // Motor state
    float current_positions[4];
    float target_positions[4];
    float velocity_command[2];  // [linear_x, angular_z]
    
    // Safety state
    bool emergency_stopped;
    unsigned long last_update;
    
    // PWM channels
    int pwm_channels[4];
    
    /**
     * @brief Read ADC value with filtering
     * 
     * @param motor_index Motor index (0-3)
     * @return float Filtered ADC value (0.0 - 1.0)
     */
    float read_adc_filtered(int motor_index);
    
    /**
     * @brief Set motor PWM with safety limits
     * 
     * @param motor_index Motor index (0-3)
     * @param speed Speed (-1.0 to 1.0)
     */
    void set_motor_speed(int motor_index, float speed);
    
    /**
     * @brief Check battery voltage
     * 
     * @return true if battery is OK
     */
    bool check_battery();
    
    /**
     * @brief Apply joint limits
     * 
     * @param position Requested position
     * @param motor_index Motor index
     * @return float Clamped position
     */
    float apply_joint_limits(float position, int motor_index);
};

#endif // MOTOR_CONTROL_H
