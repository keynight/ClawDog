# ClawDog - Agent Context

## Project Overview

ClawDog is an autonomous quadruped robot project converting a cheap toy robot dog into a fully autonomous system using ROS2, ESP32-S3, and OpenClaw. The project is currently in early conceptual/development phase.

**Key Technologies:**
- ESP32-S3-H16R8 (8MB PSRAM) - Real-time motor control, sensor reading
- ROS2 Humble/Jazzy - Navigation, SLAM, high-level control
- OpenClaw - Quadruped gait generation and AI integration
- WitMotion D6 LiDAR - SLAM and obstacle avoidance
- **LD2420 mmWave Radar** - Human presence detection, emergency stop trigger
- **0.96" OLED Display (128×64, I2C, SSD1306)** - Status display, battery, gait mode, errors
- SA8301 H-bridge motor drivers
- PlatformIO / ESP-IDF for ESP32 firmware

## Repository Structure (Planned)

```
ClawDog/
├── firmware/                 # ESP32-S3 firmware
│   ├── src/                 # Source code
│   ├── include/             # Headers
│   ├── lib/                 # Custom libraries
│   ├── test/                # Unit tests
│   └── platformio.ini       # PlatformIO configuration
├── ros2_ws/                  # ROS2 workspace
│   ├── src/
│   │   ├── clawdog_control/     # ros2_control hardware interface
│   │   ├── clawdog_description/ # URDF, meshes
│   │   ├── clawdog_bringup/     # Launch files
│   │   └── witmotion_driver/    # LiDAR driver
├── openclaw/                 # OpenClaw integration
│   ├── config/
│   └── src/
├── docs/                     # Documentation
│   ├── decisions/           # ADRs
│   └── hardware/            # Hardware docs, pinouts
├── scripts/                  # Utility scripts
├── tests/                    # Integration tests
├── DESC.md                   # Project description
├── README.md                 # User-facing readme
└── CLAUDE.md                 # This file
```

## Critical Safety Rules

### Hardware Safety (MANDATORY)
- **Always assume motors can move unexpectedly** - code must have emergency stop capability
- **PWM duty cycle limits** - never exceed 80% sustained to prevent motor/driver damage
- **Joint limits** - all motion commands must respect calibrated ADC min/max values
- **Watchdog timer** - ESP32 must stop all motors if ROS2 heartbeat >100ms
- **Low battery cutoff** - force sit pose if battery <6.0V (2S LiPo)
- **Emergency stop** - physical button must cut motor power independently of software
- **Human presence detection** - LD2420 mmWave radar triggers emergency stop when humans detected in unsafe proximity during autonomous operation

### Code Safety Patterns
- Every motor control function must check joint limits before execution
- All motion commands must have timeout/failsafe behavior
- Never disable watchdog or safety checks in production code
- Test all new code with robot **suspended** (no ground contact) first
- Log all safety events (limits hit, timeouts, emergency stops)

## Development Conventions

### ESP32 Firmware
- **Framework**: Arduino framework via PlatformIO (easier libraries) or ESP-IDF (performance)
- **Task separation**: 
  - Core 0: Communication (WiFi, UDP, ROS2 bridge)
  - Core 1: Real-time control (PID, PWM, ADC)
- **PID loops**: Run at 200Hz minimum for stable joint control
- **UDP protocol**: Binary, 50Hz update rate
  - TX: `[joint0, joint1, joint2, joint3, roll, pitch, yaw]` (7 floats)
  - RX: `[target0, target1, target2, target3]` (4 floats)
- **ADC reads**: 12-bit, with moving average filter (5-10 samples)

### ROS2 Code
- **Distribution**: Humble (Ubuntu 22.04) or Jazzy (Ubuntu 24.04)
- **Build tool**: colcon
- **Language**: C++ for real-time nodes, Python for tooling
- **Naming**: Package names use `clawdog_*` prefix
- **Topics**:
  - `/joint_states` - Current joint positions (sensor_msgs/JointState)
  - `/joint_trajectory` - Target positions (trajectory_msgs/JointTrajectory)
  - `/scan` - LiDAR data (sensor_msgs/LaserScan)
  - `/cmd_vel` - Velocity commands (geometry_msgs/Twist)
  - `/odom` - Odometry (nav_msgs/Odometry)

### Git Workflow
- **Main branch**: `main` - stable, tested code only
- **Feature branches**: `feature/<description>`
- **Hardware-specific branches**: `hw/<version>` for board revisions
- **Commits**: Use conventional commits format
- **No force push to main**

## Key Design Decisions

### Why UDP instead of micro-ROS
- micro-ROS adds complexity and memory overhead on ESP32
- UDP binary protocol is simpler, lower latency, and sufficient for our needs
- Can be replaced with micro-ROS later if needed

### Why ROS2 on companion computer, not ESP32
- ESP32 lacks resources for full ROS2 + SLAM + Nav2
- Separation of concerns: real-time control vs high-level planning
- WiFi bridge provides flexibility in companion computer choice

### Why OpenClaw
- Established quadruped gait generation framework
- Already handles trot, walk, stand gaits
- ROS2 integration available

## Pin Mapping (ESP32-S3)

| Function | GPIO | Notes |
|----------|------|-------|
| Motor 0 PWM | 1 | |
| Motor 0 DIR | 2 | |
| Motor 0 ADC | 13 | Potentiometer (GPIO 3 is strapping pin) |
| Motor 1 PWM | 4 | |
| Motor 1 DIR | 5 | |
| Motor 1 ADC | 6 | |
| Motor 2 PWM | 7 | |
| Motor 2 DIR | 8 | |
| Motor 2 ADC | 9 | |
| Motor 3 PWM | 10 | |
| Motor 3 DIR | 11 | |
| Motor 3 ADC | 12 | |
| UART2 RX | 17 | LiDAR |
| UART2 TX | 18 | LiDAR |
| UART1 RX | 15 | mmWave Radar (LD2420) |
| UART1 TX | 16 | mmWave Radar (LD2420) |
| I2C SDA | 40 | IMU (avoided 10 due to conflict) |
| I2C SCL | 41 | IMU (avoided 11 due to conflict) |

**⚠️ Avoid strapping pins**: GPIO 0, 3, 46, etc.

## Calibration Values (To be determined)

These must be measured per-robot and stored in firmware config:

| Joint | ADC Min | ADC Max | Angle Min (deg) | Angle Max (deg) |
|-------|---------|---------|-----------------|-----------------|
| 0 | TBD | TBD | TBD | TBD |
| 1 | TBD | TBD | TBD | TBD |
| 2 | TBD | TBD | TBD | TBD |
| 3 | TBD | TBD | TBD | TBD |

## Known Gotchas

### Hardware
- **SA8301 inputs are 3.3V logic** - verify before connecting to ESP32
- **Potentiometers may have dead zones** - calibrate carefully, add software deadband if needed
- **Motor noise can affect ADC readings** - separate analog/digital grounds, add capacitors
- **I2C pull-ups required** for BNO055 (4.7kΩ typical)
- **LiDAR requires clean 5V** - use dedicated buck converter, not shared rail

### Software
- **PWM frequency**: Start with 1kHz, adjust based on motor response
- **PID tuning**: Start conservative (low P, zero I/D), increase gradually
- **UDP packet loss**: Add sequence numbers, handle missing frames gracefully
- **ROS2 discovery**: Can be slow over WiFi - consider Fast DDS tuning
- **Gait timing**: Quadruped gaits are sensitive to timing - maintain consistent loop rate

## Development Phases (In Order)

1. **Suspended test** - Verify joints move, ADC reads correctly
2. **PID tuning** - One leg at a time, hold position
3. **UDP communication** - Loopback test with companion PC
4. **OpenClaw simulation** - Test gait generation
5. **On-floor crawling** - Static gait first
6. **IMU feedback** - Tilt compensation
7. **LiDAR + SLAM** - Build map while teleoperating
8. **Nav2 autonomous** - Full autonomous navigation

## External Resources

- **OpenClaw**: https://github.com/openclaw (quadruped framework)
- **ROS2 Documentation**: https://docs.ros.org/en/humble/
- **ESP32-S3 Technical Reference**: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/
- **SA8301 Datasheet**: Search "SA8301 H-bridge datasheet"
- **WitMotion D6 Protocol**: Contact WitMotion or reverse engineer from SDK

## License

Apache License 2.0 - See LICENSE file

---

*This file is maintained for AI agents working on the project. Update when architectural decisions change or new gotchas are discovered.*
