# ClawDog System Architecture

## Overview

ClawDog is a low-cost autonomous quadruped robot built from a toy robot dog. The system follows a **分层架构 (layered architecture)** with clear separation between real-time control and high-level intelligence.

```
┌─────────────────────────────────────────────────────────────┐
│                    AI / User Interface Layer                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Telegram   │  │  WhatsApp   │  │  Direct API/CLI     │  │
│  │  Bot        │  │  Bot        │  │                     │  │
│  └──────┬──────┘  └──────┬──────┘  └──────────┬──────────┘  │
└─────────┼────────────────┼────────────────────┼─────────────┘
          │                │                    │
          └────────────────┴────────────────────┘
                              │
                    ┌─────────▼─────────┐
                    │    RosClaw        │
                    │  (AI Agent Bridge)│
                    └─────────┬─────────┘
                              │
┌─────────────────────────────▼───────────────────────────────┐
│                 High-Level Control Layer                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  OpenClaw   │  │    Nav2     │  │  SLAM Toolbox       │  │
│  │  (Gaits)    │  │ (Planning)  │  │  (Mapping)          │  │
│  └──────┬──────┘  └──────┬──────┘  └──────────┬──────────┘  │
└─────────┼────────────────┼────────────────────┼─────────────┘
          │                │                    │
          └────────────────┴────────────────────┘
                              │
                    ┌─────────▼─────────┐
                    │   ros2_control    │
                    │  (Hardware IF)    │
                    └─────────┬─────────┘
                              │ UDP (50Hz)
┌─────────────────────────────▼───────────────────────────────┐
│                Real-Time Control Layer                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   PID x4    │  │    IMU      │  │  LiDAR Forwarding   │  │
│  │  (200Hz)    │  │ (BNO055)    │  │  (WitMotion D6)     │  │
│  └──────┬──────┘  └──────┬──────┘  └──────────┬──────────┘  │
│         │                │                    │             │
│  ┌──────▼──────┐  ┌──────▼──────┐  ┌──────────▼──────────┐  │
│  │   Motors    │  │  ADC x4     │  │    UART2            │  │
│  │ (SA8301)    │  │(Pots)       │  │   (921600 baud)     │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  mmWave     │  │    OLED     │  │                     │  │
│  │  (LD2420)   │  │  (SSD1306)  │  │                     │  │
│  │ Human Det.  │  │  Status     │  │                     │  │
│  └──────┬──────┘  └──────┬──────┘  │                     │  │
│         │                │         │                     │  │
│  ┌──────▼──────┐  ┌──────▼──────┐  │                     │  │
│  │   UART1     │  │    I2C      │  │                     │  │
│  │ (115200)    │  │  (400kHz)   │  │                     │  │
│  └─────────────┘  └─────────────┘  │                     │  │
│                                                              │
│                    ESP32-S3-N16R8                            │
└─────────────────────────────────────────────────────────────┘
```

## Layer Responsibilities

### 1. Real-Time Control Layer (ESP32-S3)
**Location**: On-robot microcontroller
**Constraints**: 200Hz control loops, <1ms jitter, safety-critical

Responsibilities:
- **Motor Control**: 4× PID loops for leg positioning
- **Sensor Reading**: ADC (potentiometers), I2C (IMU, OLED), UART (LiDAR, mmWave)
- **Safety**: Watchdog timer, joint limits, emergency stop, human presence detection
- **Communication**: UDP bridge to companion computer
- **Display**: Status OLED (battery, gait, errors)

Key Files (when implemented):
- `firmware/src/main.cpp` - Main control loop
- `firmware/src/pid_controller.cpp` - PID implementation
- `firmware/src/motor_driver.cpp` - SA8301 PWM control
- `firmware/src/udp_bridge.cpp` - WiFi communication
- `firmware/src/safety.cpp` - Watchdog and limits
- `firmware/src/radar.cpp` - LD2420 human detection
- `firmware/src/display.cpp` - SSD1306 OLED status

### 2. High-Level Control Layer (Companion Computer)
**Location**: RPi5 or Jetson Orin Nano
**Constraints**: Real-time not required, <100ms acceptable latency

Responsibilities:
- **ROS2 Core**: Node management, topic pub/sub, parameter server
- **Navigation**: Nav2 global/local planners, costmaps
- **Mapping**: SLAM Toolbox for 2D occupancy grids
- **Gait Generation**: OpenClaw for trot/walk/stand gaits
- **Hardware Interface**: UDP bridge to ESP32

Key Files (when implemented):
- `ros2_ws/src/clawdog_control/` - Hardware interface
- `ros2_ws/src/clawdog_bringup/` - Launch files
- `ros2_ws/src/witmotion_driver/` - LiDAR ROS2 node

### 3. AI / User Interface Layer
**Location**: Companion computer (same or remote)
**Constraints**: Non-real-time, async processing

Responsibilities:
- **Natural Language**: Parse user commands ("go to kitchen")
- **Task Planning**: Decompose high-level goals into ROS2 actions
- **Safety Checks**: Verify commands before execution
- **Audit Logging**: Record all AI decisions and user commands

Key Files (when implemented):
- `openclaw/src/rosclaw_bridge.py` - AI agent integration
- `openclaw/config/capabilities.yaml` - Robot capability definition

## Data Flow

### Sensor Data Flow (Upstream)
```
Potentiometers → ADC → ESP32 → UDP → ros2_control → /joint_states
IMU (BNO055)   → I2C → ESP32 → UDP → ros2_control → /imu/data
LiDAR (D6)     → UART2 → ESP32 → UDP → witmotion_driver → /scan
mmWave (LD2420) → UART1 → ESP32 → UDP → safety_monitor → /human_presence
```

### Command Flow (Downstream)
```
User Command → RosClaw → OpenClaw → Nav2 → ros2_control → UDP → ESP32 → Motors
                     ↓
              joint_trajectory_controller → target angles
```

### Safety Flow
```
Emergency Stop Button → Hardware relay (cuts motor power)
Human Detected (LD2420) → ESP32 emergency stop + log
Watchdog Timeout → ESP32 sets PWM=0, enters safe state
Low Battery → ESP32 forces sit pose, disables movement
Joint Limit → Software stop, log event
```

## Component Interactions

### ESP32 ↔ Companion Computer (UDP Protocol)
- **Frequency**: 50Hz (20ms period)
- **TX Frame** (7 floats): `[joint0, joint1, joint2, joint3, roll, pitch, yaw]`
- **RX Frame** (4 floats): `[target0, target1, target2, target3]`
- **Port**: 8888 (configurable)
- **Watchdog**: ESP32 stops motors if no RX frame for >100ms

### ROS2 Topic Overview
| Topic | Type | Publisher | Subscriber | Rate |
|-------|------|-----------|------------|------|
| `/joint_states` | sensor_msgs/JointState | clawdog_control | OpenClaw, Nav2 | 50Hz |
| `/joint_trajectory` | trajectory_msgs/JointTrajectory | OpenClaw | clawdog_control | 50Hz |
| `/scan` | sensor_msgs/LaserScan | witmotion_driver | SLAM, Nav2 | 10Hz |
| `/cmd_vel` | geometry_msgs/Twist | Nav2 | clawdog_control | 20Hz |
| `/odom` | nav_msgs/Odometry | SLAM/Nav2 | OpenClaw | 20Hz |
| `/imu/data` | sensor_msgs/Imu | clawdog_control | OpenClaw | 50Hz |
| `/human_presence` | std_msgs/Bool | clawdog_safety | Nav2, RosClaw | 10Hz |

## Hardware Architecture

### Power Tree
```
2S LiPo 7.4V
    ├───→ SA8301 Motor Drivers (direct, fused)
    ├───→ MP1584 Buck Converter ──→ 5V/3A ──→ WitMotion D6 LiDAR, LD2420 (if 5V variant)
    └───→ AMS1117-3.3 LDO ──→ 3.3V ──→ ESP32-S3, BNO055, SSD1306 OLED, LD2420 (if 3.3V variant)
```

**Critical**: Common ground for all subsystems. Separate ground planes for analog (sensors) and digital (ESP32) to reduce noise.

### Motor Control Chain
```
ESP32 GPIO (PWM+DIR) → SA8301 H-Bridge → DC Motor → Leg Movement
                                      ↑
                              Potentiometer Feedback → ESP32 ADC
```

## Development State

This project is in **early conceptual phase**. Current status:

- [x] Hardware reverse engineering planned
- [x] Software architecture defined
- [x] Communication protocol specified
- [ ] Firmware implementation
- [ ] ROS2 package implementation
- [ ] Hardware modifications
- [ ] Integration testing

See README.md for detailed development phases and timeline.

## Getting Started (For Agents)

When implementing features:
1. Read CLAUDE.md for coding conventions and safety rules
2. Check ADRs in `docs/decisions/` for architectural context
3. Follow the development phase order (don't skip ahead)
4. Always include safety checks in motor control code
5. Test with robot suspended (no ground contact) first
6. Document any new gotchas or hardware-specific findings

## References

- CLAUDE.md - Agent rules and conventions
- DESC.md - Full project description
- docs/decisions/ - Architecture Decision Records
- README.md - User-facing project overview
