/**
 * @file motor_control.cpp
 * @brief Motor control implementation for ClawDog quadruped robot
 * 
 * Implements MotorControl class with safety limits and position feedback.
 */

#include "motor_control.h"

// ======================== CONSTRUCTOR ========================

MotorControl::MotorControl() 
    : emergency_stopped(false)
    , last_update(0) {
    for (int i = 0; i < 4; i++) {
        current_positions[i] = 0.0f;
        target_positions[i] = 0.0f;
        pwm_channels[i] = i;
    }
    velocity_command[0] = 0.0f;
    velocity_command[1] = 0.0f;
}

// ======================== PUBLIC METHODS ========================

void MotorControl::init() {
    Serial.println("[Motor] Initializing motor control...");
    
    // Configure PWM channels
    for (int i = 0; i < 4; i++) {
        ledcSetup(pwm_channels[i], PWM_FREQ, PWM_RESOLUTION);
        ledcAttachPin(MOTOR_PINS[i].pwm, pwm_channels[i]);
        
        pinMode(MOTOR_PINS[i].dir, OUTPUT);
        digitalWrite(MOTOR_PINS[i].dir, LOW);
        
        ledcWrite(pwm_channels[i], 0);
    }
    
    // Configure ADC
    analogReadResolution(ADC_RESOLUTION);
    analogSetAttenuation(ADC_11db);
    
    Serial.println("[Motor] Motor control initialized");
}

void MotorControl::update() {
    if (emergency_stopped) {
        return;
    }
    
    // Check battery
    if (!check_battery()) {
        Serial.println("[Motor] Battery low! Emergency stopping...");
        emergency_stop();
        return;
    }
    
    // Read current positions
    for (int i = 0; i < 4; i++) {
        current_positions[i] = read_adc_filtered(i);
    }
    
    // TODO: Implement PID control
    // For now, just hold position or follow simple velocity command
    if (abs(velocity_command[0]) > 0.01 || abs(velocity_command[1]) > 0.01) {
        // Simple open-loop velocity control for testing
        float speed = velocity_command[0] * MAX_PWM_PERCENT;
        for (int i = 0; i < 4; i++) {
            set_motor_speed(i, speed);
        }
    } else {
        // Hold position - stop motors
        for (int i = 0; i < 4; i++) {
            set_motor_speed(i, 0.0f);
        }
    }
    
    last_update = millis();
}

void MotorControl::set_velocity_command(float linear_x, float angular_z) {
    velocity_command[0] = constrain(linear_x, -1.0f, 1.0f);
    velocity_command[1] = constrain(angular_z, -1.0f, 1.0f);
}

void MotorControl::get_joint_positions(float positions[4]) {
    for (int i = 0; i < 4; i++) {
        positions[i] = current_positions[i];
    }
}

float MotorControl::get_battery_voltage() {
    // Read battery voltage from ADC
    // Assuming a voltage divider connected to one of the ADC pins
    // For now, return a placeholder value
    int raw = analogRead(14);  // Battery ADC pin (adjust as needed)
    float voltage = (raw / (float)ADC_MAX_VALUE) * ADC_VOLTAGE_REF * BATTERY_ADC_DIVIDER;
    return voltage;
}

void MotorControl::emergency_stop() {
    Serial.println("[Motor] EMERGENCY STOP!");
    emergency_stopped = true;
    
    for (int i = 0; i < 4; i++) {
        ledcWrite(pwm_channels[i], 0);
        digitalWrite(MOTOR_PINS[i].dir, LOW);
    }
}

void MotorControl::resume() {
    Serial.println("[Motor] Resuming...");
    emergency_stopped = false;
}

// ======================== PRIVATE METHODS ========================

float MotorControl::read_adc_filtered(int motor_index) {
    // Simple moving average filter
    const int samples = 5;
    long sum = 0;
    
    for (int i = 0; i < samples; i++) {
        sum += analogRead(MOTOR_PINS[motor_index].adc);
        delayMicroseconds(100);
    }
    
    return (sum / (float)samples) / ADC_MAX_VALUE;
}

void MotorControl::set_motor_speed(int motor_index, float speed) {
    speed = constrain(speed, -MAX_PWM_PERCENT, MAX_PWM_PERCENT);
    
    if (abs(speed) < 0.01f) {
        ledcWrite(pwm_channels[motor_index], 0);
        return;
    }
    
    digitalWrite(MOTOR_PINS[motor_index].dir, speed > 0 ? HIGH : LOW);
    
    int duty = abs(speed) * PWM_MAX_DUTY;
    ledcWrite(pwm_channels[motor_index], duty);
}

bool MotorControl::check_battery() {
    float voltage = get_battery_voltage();
    return voltage > BATTERY_MIN_VOLTAGE;
}

float MotorControl::apply_joint_limits(float position, int motor_index) {
    // TODO: Implement joint limits based on calibration
    // For now, just constrain to valid range
    return constrain(position, 0.0f, 1.0f);
}