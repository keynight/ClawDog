# ADR-002: Run ROS2 on Companion Computer Instead of ESP32

## Status
Accepted

## Date
2025-01-20

## Context
The ClawDog robot requires:
- Real-time motor control (PID loops at 200Hz)
- SLAM (Simultaneous Localization and Mapping) from LiDAR data
- Global and local path planning (Nav2)
- High-level AI task planning (OpenClaw)
- Natural language command processing (RosClaw)

We must decide where to run ROS2: on the ESP32-S3 microcontroller, on a companion computer, or distributed across both.

## Decision
Run the full ROS2 stack on a companion computer (RPi5 or Jetson Orin Nano), with the ESP32-S3 handling only real-time motor control and sensor reading.

Architecture:
```
┌─────────────────────────────────┐
│  Companion Computer (RPi5/      │
│   Jetson Orin Nano)             │
│  - ROS2 Humble/Jazzy            │
│  - Nav2 navigation              │
│  - SLAM Toolbox                 │
│  - OpenClaw gait generation     │
│  - RosClaw AI bridge            │
├─────────────────────────────────┤
│  UDP Bridge Node                │
├─────────────────────────────────┤
│  WiFi                           │
└─────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│  ESP32-S3-H16R8                 │
│  - PID motor control (200Hz)    │
│  - ADC reading (potentiometers) │
│  - LiDAR data forwarding        │
│  - IMU reading                  │
│  - UDP communication (50Hz)     │
└─────────────────────────────────┘
```

## Alternatives Considered

### micro-ROS on ESP32
- **Pros**: Single ROS2 domain, no custom protocol needed
- **Cons**: 
  - ESP32 lacks RAM/CPU for full ROS2 stack
  - Cannot run Nav2, SLAM, or OpenClaw on ESP32
  - micro-ROS is client-only; still needs agent on PC
- **Rejected**: ESP32 cannot handle computationally expensive ROS2 nodes

### Split ROS2 (some nodes on ESP32, some on PC)
- **Pros**: Distributes load, keeps some ROS2 logic on the robot
- **Cons**: 
  - Complex network topology
  - ESP32 still can't run heavy nodes
  - Debugging distributed systems is harder
- **Rejected**: Added complexity without benefit - ESP32 is best as a "dumb" real-time controller

### No ROS2 (custom stack on ESP32)
- **Pros**: Minimal overhead, no companion computer needed
- **Cons**: 
  - Must reimplement SLAM, navigation, planning from scratch
  - No ecosystem of ROS2 packages
  - Harder to integrate AI/LLM capabilities
- **Rejected**: ROS2 ecosystem provides battle-tested navigation and planning

## Consequences

**Positive:**
- ESP32 firmware is simple and focused (motor control + sensors)
- Companion computer has ample resources for ROS2, SLAM, and AI
- Can upgrade companion computer independently of ESP32
- Easy to debug ROS2 nodes on a standard Linux machine
- WiFi bridge allows remote operation and monitoring

**Negative:**
- Requires companion computer (adds cost, power consumption, weight)
- WiFi dependency for autonomous operation
- UDP bridge introduces ~10-20ms latency
- More complex system with two computers to manage

**Mitigations:**
- Use wired Ethernet if WiFi is unreliable (future enhancement)
- Implement watchdog on ESP32 for communication loss
- Keep ESP32 firmware simple enough to be maintainable
- Use lightweight companion computer (RPi5 is ~$80)

## References
- DESC.md section 3.2 for ROS2 software stack details
- ROS2 Humble documentation: https://docs.ros.org/en/humble/
- ADR-001 for UDP protocol specification
