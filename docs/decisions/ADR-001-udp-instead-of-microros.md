# ADR-001: Use UDP Binary Protocol Instead of micro-ROS for ESP32 Communication

## Status
Accepted

## Date
2025-01-20

## Context
We need a communication protocol between the ESP32-S3 (real-time motor control) and the companion computer (ROS2). The ESP32-S3-H16R8 has 8MB PSRAM and must handle:
- 4x PID motor controllers at 200Hz
- LiDAR data streaming via UART2
- IMU readings via I2C
- WiFi connectivity

Key requirements:
- Low latency (<20ms round-trip for control commands)
- Minimal memory overhead on ESP32
- Simple implementation to reduce firmware complexity
- Ability to bridge to ROS2 topics on the companion computer

## Decision
Use a lightweight UDP binary protocol instead of micro-ROS.

Protocol specification:
- **Transport**: UDP over WiFi
- **Frequency**: 50Hz update rate
- **TX Frame** (ESP32 → PC): `[joint0, joint1, joint2, joint3, roll, pitch, yaw]` (7 × float32)
- **RX Frame** (PC → ESP32): `[target0, target1, target2, target3]` (4 × float32)
- **Port**: Configurable (default 8888)

## Alternatives Considered

### micro-ROS
- **Pros**: Native ROS2 integration, standard message types, QoS support
- **Cons**: 
  - Adds ~100KB+ flash and ~50KB+ RAM overhead on ESP32
  - Complex build system requiring micro-ROS agent on PC
  - Slower discovery and serialization than raw binary
  - Adds failure modes (agent crashes, DDS discovery issues)
- **Rejected**: Memory overhead and complexity not justified for simple joint angle streaming

### MQTT
- **Pros**: Lightweight, pub/sub model, wide library support
- **Cons**: Requires MQTT broker, text-based JSON serialization adds latency
- **Rejected**: Adds broker dependency and serialization overhead

### TCP/Custom Protocol
- **Pros**: Reliable delivery, can add sequence numbers and checksums
- **Cons**: Higher latency than UDP, more complex state management
- **Rejected**: UDP is sufficient for real-time control; packet loss handled gracefully by next frame

## Consequences

**Positive:**
- Minimal firmware footprint (~2KB for UDP stack vs ~150KB for micro-ROS)
- Simple C++ implementation using Arduino WiFiUdp or ESP-IDF sockets
- Easy to debug with netcat or Wireshark
- Can be replaced with micro-ROS later without changing motor control logic

**Negative:**
- Must implement ROS2 topic bridging manually on companion computer
- No built-in QoS or reliability guarantees
- Need to handle packet loss and out-of-order delivery in application logic
- Less "standard" - requires custom documentation

**Mitigations:**
- Add sequence numbers to UDP frames for detecting packet loss
- Implement heartbeat/watchdog on ESP32 (stop motors if no RX frames for >100ms)
- Create simple Python bridge node on companion computer that converts UDP to ROS2 topics

## References
- [micro-ROS documentation](https://micro.ros.org/)
- ESP32 WiFiUdp library examples
- DESC.md section 3.1 for protocol details
