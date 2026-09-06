/**
 * @file main.cpp
 * @brief ClawDog micro-ROS Firmware
 * 
 * ESP32-S3 based quadruped robot firmware with micro-ROS over WiFi UDP.
 * Implements reconnection logic, cmd_vel subscriber, and joint states publisher.
 * 
 * Based on patterns from micro-ROS ESP32 examples:
 * - Reconnection logic from bcanozter/microROS-ESP32
 * - Motor control with safety limits
 * 
 * @version 1.0.0
 * @date 2026-05-21
 * 
 * @copyright Copyright (c) 2026
 * 
 * Hardware: ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM)
 * Framework: Arduino via PlatformIO
 */

#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <WiFi.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <geometry_msgs/msg/twist.h>
#include <sensor_msgs/msg/joint_state.h>
#include <std_msgs/msg/float32.h>
#include <std_msgs/msg/bool.h>

#include "motor_control.h"

// ======================== CONFIGURATION ========================

// WiFi Credentials - Override via build_flags or change here
#ifndef WIFI_SSID
  #define WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASS
  #define WIFI_PASS "YOUR_WIFI_PASSWORD"
#endif

// micro-ROS Agent Configuration
#ifndef AGENT_IP
  #define AGENT_IP "192.168.1.100"
#endif
#ifndef AGENT_PORT
  #define AGENT_PORT "8888"
#endif

// Timing Configuration
#define LOOP_RATE_MS 50        // Main loop rate (20Hz)
#define AGENT_TIMEOUT_MS 1000  // Agent connection timeout
#define RECONNECT_DELAY_MS 2000 // Delay between reconnection attempts

// ======================== MACROS ========================

// Error checking macros
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) { error_loop(); } }
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) { } }

// ======================== GLOBAL STATE ========================

enum class AgentState {
    WAITING_FOR_AGENT,
    CONNECTED,
    DISCONNECTED
};

AgentState agent_state = AgentState::WAITING_FOR_AGENT;

// ROS2 Entities
rclc_support_t support;
rcl_allocator_t allocator;
rcl_init_options_t init_options;
rcl_node_t node;
rclc_executor_t executor;

// Publishers
rcl_publisher_t joint_state_pub;
rcl_publisher_t battery_pub;
rcl_publisher_t imu_pub;
rcl_publisher_t heartbeat_pub;

// Subscribers
rcl_subscription_t cmd_vel_sub;

// Messages
sensor_msgs__msg__JointState joint_state_msg;
geometry_msgs__msg__Twist cmd_vel_msg;
std_msgs__msg__Float32 battery_msg;
std_msgs__msg__Bool heartbeat_msg;

// Timing
unsigned long last_heartbeat = 0;
unsigned long last_agent_ping = 0;
unsigned long reconnect_timer = 0;

// Motor control instance
MotorControl motors;

// ======================== CALLBACKS ========================

/**
 * @brief cmd_vel subscriber callback - receives velocity commands from ROS2 teleop
 * 
 * Maps Twist messages to motor commands for differential/omni drive control.
 * For quadruped, this will be mapped to gait parameters.
 */
void cmd_vel_callback(const void *msg_in) {
    const geometry_msgs__msg__Twist *msg = (const geometry_msgs__msg__Twist *)msg_in;
    
    // Log received command
    Serial.printf("[CMD_VEL] linear: %.2f, %.2f, %.2f | angular: %.2f, %.2f, %.2f\n",
                  msg->linear.x, msg->linear.y, msg->linear.z,
                  msg->angular.x, msg->angular.y, msg->angular.z);
    
    // TODO: Map to gait parameters
    // For now, store for motor control loop
    motors.set_velocity_command(msg->linear.x, msg->angular.z);
}

// ======================== SETUP FUNCTIONS ========================

/**
 * @brief Initialize WiFi connection
 */
bool init_wifi() {
    Serial.println("[WiFi] Connecting to " + String(WIFI_SSID) + "...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());
        return true;
    } else {
        Serial.println("\n[WiFi] Connection failed!");
        return false;
    }
}

/**
 * @brief Initialize micro-ROS transport (WiFi UDP)
 */
bool init_microros_transport() {
    Serial.println("[micro-ROS] Initializing WiFi UDP transport...");
    
    // Set agent IP and port
    rmw_init_options_t *rmw_options = rcl_init_options_get_rmw_init_options(&init_options);
    rmw_uros_options_set_udp_address(AGENT_IP, AGENT_PORT, rmw_options);
    
    Serial.printf("[micro-ROS] Agent: %s:%s\n", AGENT_IP, AGENT_PORT);
    return true;
}

/**
 * @brief Initialize ROS2 entities (node, publishers, subscribers, executor)
 */
bool create_entities() {
    Serial.println("[micro-ROS] Creating ROS2 entities...");
    
    // Initialize allocator
    allocator = rcl_get_default_allocator();
    
    // Initialize init options
    RCCHECK(rcl_init_options_init(&init_options, allocator));
    
    // Initialize transport
    init_microros_transport();
    
    // Initialize support
    RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));
    
    // Create node
    RCCHECK(rclc_node_init_default(&node, "clawdog_node", "", &support));
    
    // ======================== PUBLISHERS ========================
    
    // Joint States Publisher
    RCCHECK(rclc_publisher_init_default(
        &joint_state_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
        "joint_states"));
    
    // Battery Publisher  
    RCCHECK(rclc_publisher_init_default(
        &battery_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        "battery_voltage"));
    
    // IMU Publisher (placeholder)
    RCCHECK(rclc_publisher_init_default(
        &imu_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
        "imu_orientation"));
    
    // Heartbeat Publisher
    RCCHECK(rclc_publisher_init_default(
        &heartbeat_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        "heartbeat"));
    
    // ======================== SUBSCRIBERS ========================
    
    // cmd_vel Subscriber
    RCCHECK(rclc_subscription_init_default(
        &cmd_vel_sub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
        "cmd_vel"));
    
    // ======================== EXECUTOR ========================
    
    // Initialize executor with 1 handle
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    
    // Add subscriptions
    RCCHECK(rclc_executor_add_subscription(
        &executor, &cmd_vel_sub, &cmd_vel_msg, &cmd_vel_callback, ON_NEW_DATA));
    
    Serial.println("[micro-ROS] Entities created successfully");
    return true;
}

/**
 * @brief Destroy all ROS2 entities (for reconnection)
 */
void destroy_entities() {
    Serial.println("[micro-ROS] Destroying entities...");
    
    // Clean up subscriptions
    rcl_subscription_fini(&cmd_vel_sub, &node);
    
    // Clean up publishers
    rcl_publisher_fini(&joint_state_pub, &node);
    rcl_publisher_fini(&battery_pub, &node);
    rcl_publisher_fini(&imu_pub, &node);
    rcl_publisher_fini(&heartbeat_pub, &node);
    
    // Clean up executor
    rclc_executor_fini(&executor);
    
    // Clean up node
    rcl_node_fini(&node);
    
    // Clean up support
    rclc_support_fini(&support);
    
    // Clean up init options
    rcl_init_options_fini(&init_options);
    
    Serial.println("[micro-ROS] Entities destroyed");
}

/**
 * @brief Error handler - blink LED in pattern
 */
void error_loop() {
    Serial.println("[ERROR] Critical error occurred!");
    while (1) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(100);
    }
}

/**
 * @brief Initialize joint state message memory
 */
void init_joint_state_message() {
    // Allocate memory for joint state message
    // For 4 joints (quadruped leg DOF)
    const int num_joints = 4;
    
    joint_state_msg.name.capacity = num_joints;
    joint_state_msg.name.size = num_joints;
    joint_state_msg.name.data = (rosidl_runtime_c__String *)malloc(num_joints * sizeof(rosidl_runtime_c__String));
    
    joint_state_msg.position.capacity = num_joints;
    joint_state_msg.position.size = num_joints;
    joint_state_msg.position.data = (double *)malloc(num_joints * sizeof(double));
    
    joint_state_msg.velocity.capacity = num_joints;
    joint_state_msg.velocity.size = num_joints;
    joint_state_msg.velocity.data = (double *)malloc(num_joints * sizeof(double));
    
    joint_state_msg.effort.capacity = num_joints;
    joint_state_msg.effort.size = num_joints;
    joint_state_msg.effort.data = (double *)malloc(num_joints * sizeof(double));
    
    // Set joint names
    const char *joint_names[] = {"joint_0", "joint_1", "joint_2", "joint_3"};
    for (int i = 0; i < num_joints; i++) {
        joint_state_msg.name.data[i].data = (char *)malloc(strlen(joint_names[i]) + 1);
        joint_state_msg.name.data[i].capacity = strlen(joint_names[i]) + 1;
        strcpy(joint_state_msg.name.data[i].data, joint_names[i]);
        joint_state_msg.name.data[i].size = strlen(joint_names[i]);
    }
    
    // Initialize header frame
    joint_state_msg.header.frame_id.data = (char *)malloc(10);
    joint_state_msg.header.frame_id.capacity = 10;
    strcpy(joint_state_msg.header.frame_id.data, "base_link");
    joint_state_msg.header.frame_id.size = 9;
}

// ======================== PUBLISH FUNCTIONS ========================

/**
 * @brief Publish joint states from motor encoders/ADC
 */
void publish_joint_states() {
    // Read motor positions from ADC
    float positions[4];
    motors.get_joint_positions(positions);
    
    // Update timestamp
    int64_t time_ns = rmw_uros_epoch_nanos();
    joint_state_msg.header.stamp.sec = time_ns / 1000000000;
    joint_state_msg.header.stamp.nanosec = time_ns % 1000000000;
    
    // Update positions
    for (int i = 0; i < 4; i++) {
        joint_state_msg.position.data[i] = positions[i];
        joint_state_msg.velocity.data[i] = 0.0;  // TODO: compute velocity
        joint_state_msg.effort.data[i] = 0.0;    // TODO: compute effort
    }
    
    RCSOFTCHECK(rcl_publish(&joint_state_pub, &joint_state_msg, NULL));
}

/**
 * @brief Publish battery voltage
 */
void publish_battery() {
    float voltage = motors.get_battery_voltage();
    battery_msg.data = voltage;
    RCSOFTCHECK(rcl_publish(&battery_pub, &battery_msg, NULL));
}

/**
 * @brief Publish heartbeat
 */
void publish_heartbeat() {
    heartbeat_msg.data = true;
    RCSOFTCHECK(rcl_publish(&heartbeat_pub, &heartbeat_msg, NULL));
}

// ======================== MAIN FUNCTIONS ========================

void setup() {
    Serial.begin(115200);
    delay(2000);  // Wait for serial monitor
    
    Serial.println("\n========================================");
    Serial.println("  ClawDog micro-ROS Firmware v1.0");
    Serial.println("  ESP32-S3-N16R8 | Quadruped Robot");
    Serial.println("========================================\n");
    
    // Initialize LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Initialize motor control
    Serial.println("[INIT] Initializing motor control...");
    motors.init();
    
    // Initialize joint state message
    init_joint_state_message();
    
    // Connect to WiFi
    if (!init_wifi()) {
        Serial.println("[ERROR] WiFi connection failed, restarting...");
        delay(5000);
        ESP.restart();
    }
    
    Serial.println("[INIT] Setup complete, waiting for micro-ROS agent...");
    Serial.printf("[INIT] Agent: %s:%s\n", AGENT_IP, AGENT_PORT);
}

void loop() {
    switch (agent_state) {
        case AgentState::WAITING_FOR_AGENT: {
            // Try to connect to agent
            if (millis() - reconnect_timer > RECONNECT_DELAY_MS) {
                reconnect_timer = millis();
                
                Serial.println("[AGENT] Attempting connection...");
                
                if (create_entities()) {
                    agent_state = AgentState::CONNECTED;
                    Serial.println("[AGENT] Connected!");
                    digitalWrite(LED_BUILTIN, HIGH);
                } else {
                    Serial.println("[AGENT] Connection failed, retrying...");
                }
            }
            break;
        }
        
        case AgentState::CONNECTED: {
            // Check agent connectivity with ping
            if (millis() - last_agent_ping > 1000) {
                last_agent_ping = millis();
                
                rmw_ret_t ping_result = rmw_uros_ping_agent(100, 3);
                
                if (ping_result != RMW_RET_OK) {
                    Serial.println("[AGENT] Connection lost!");
                    agent_state = AgentState::DISCONNECTED;
                    digitalWrite(LED_BUILTIN, LOW);
                    break;
                }
            }
            
            // Spin executor to handle subscriptions
            rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
            
            // Publish sensor data at loop rate
            static unsigned long last_publish = 0;
            if (millis() - last_publish > LOOP_RATE_MS) {
                last_publish = millis();
                
                publish_joint_states();
                publish_battery();
                publish_heartbeat();
                
                // Execute motor control loop
                motors.update();
            }
            break;
        }
        
        case AgentState::DISCONNECTED: {
            // Clean up and try to reconnect
            destroy_entities();
            reconnect_timer = millis();
            agent_state = AgentState::WAITING_FOR_AGENT;
            Serial.println("[AGENT] Reconnecting...");
            break;
        }
    }
    
    delay(10);  // Small delay to prevent watchdog issues
}
