# Project: Autonomous Robot Dog Conversion (OpenClaw + ROS2 + ESP32-S3)

## 1. Overview
Turn a cheap toy robot dog (4x DC motors + potentiometers, SA8301 H‑bridges) into a fully autonomous quadruped.  
The robot runs **ROS2** on a companion computer (RPi5/Jetson) with **OpenClaw** locomotion and **RosClaw** as the AI‑agent bridge.  
Low‑level joint control, LiDAR streaming, and IMU fusion are handled by an **ESP32‑S3** (FreeRTOS, PID, UDP bridge).

---

## 2. Hardware Architecture

| Component | Model / Spec | Role |
|-----------|--------------|------|
| Main controller | ESP32‑S3-H16R8 (8MB PSRAM) | Reads potentiometers, runs PID, sends PWM to SA8301, streams LiDAR data via UDP |
| Motor drivers | 5× SA8301 (dual H‑bridge) | Drive 4 leg motors (1 DOF per leg); extra channels unused |
| Joint feedback | Potentiometer (3‑wire) per leg | Analog voltage → ADC on ESP32 |
| LiDAR | WitMotion D6 (dTOF, 12m, 360°, UART) | SLAM and obstacle avoidance |
| mmWave | LD2420 (UART, 115200 baud) | Human tracking, emergency stop |
| IMU | BNO055 (I2C) | Orientation feedback for gait stabilisation |
| Display | SSD1306 OLED 0.96" (I2C) | Status, battery, gait mode, errors |
| Battery | 2S LiPo 7.4V 2200‑3000mAh | Powers motors directly + buck converters for 5V (LiDAR) and 3.3V (ESP32) |
| Companion computer | RPi5 / Jetson Orin Nano | ROS2, Nav2, SLAM, OpenClaw, RosClaw bridge |

### Power Tree (critical)
- **7.4V** → SA8301 motor drivers (direct, fused)  
- **7.4V → MP1584 (5V/3A)** → WitMotion D6 LiDAR, LD2420 radar (if 5V variant)  
- **7.4V → AMS1117‑3.3** → ESP32‑S3, BNO055 IMU, SSD1306 OLED  
- **Common GND** for all

---

## 3. Software Stack

### 3.1 ESP32‑S3 Firmware (PlatformIO / ESP‑IDF)
- **UART2** (921600 baud) → parse WitMotion D6 protocol → pack into UDP frames  
- **UART1** (115200 baud) → LD2420 human presence data → emergency stop / UDP  
- **I2C** (400kHz) → BNO055 IMU + SSD1306 OLED status display  
- **ADC reads** (4x potentiometers, 12‑bit) → joint angles  
- **PID controllers** (4 independent loops) → PWM + DIR signals to SA8301  
- **WiFi UDP bridge** (binary protocol, 50 Hz):  
  - TX: `[joint0, joint1, joint2, joint3, roll, pitch, yaw]` (7 floats)  
  - RX: `[target0, target1, target2, target3]` (4 floats)  
- **Watchdog**: if no ROS2 heartbeat >100 ms → emergency stop (PWM=0, sit)

### 3.2 Companion Computer – ROS2 (Humble/Jazzy on Ubuntu 22.04/24.04)
- **ros2_control** custom hardware interface that talks UDP to ESP32  
- **OpenClaw** – quadruped gait generation (trot, stand, etc.)  
- **RosClaw** – bridge between OpenClaw and AI agents (Telegram/WhatsApp or direct LLM)  
- **WitMotion D6 driver** (ROS2 node) → `/scan` topic  
- **SLAM Toolbox** → map  
- **Nav2** → global + local planner  
- **joint_trajectory_controller** → sends target angles to ESP32

### 3.3 AI Agent Integration (RosClaw)
- Dynamic discovery of robot capabilities (move, turn, take picture)  
- Natural language → ROS2 actions via messaging apps or direct API  
- Safety checks before execution (no collision, battery level, joint limits)  
- Audit logging of all commands

---

## 4. PCB Modifications (Hardware “Surgery”)

| Step | Action |
|------|--------|
| 1 | Locate SA8301 input pins (PWM/DIR from original MCU). |
| 2 | Cut PCB traces from JX3721G (main MCU) to SA8301 inputs. |
| 3 | Solder thin wires from SA8301 inputs to ESP32‑S3 GPIOs (PWM, DIR). |
| 4 | Solder potentiometer signal wires (white middle wire of leg connector) to ESP32 ADC pins. |
| 5 | Connect I2C IMU (MPU6050) to ESP32. |
| 6 | Build power distribution: 2S LiPo → MP1584 (5V for LiDAR), AMS1117 (3.3V for ESP32), direct 7.4V to SA8301. |

---

## 5. Pin Mapping (Example – Adjust after probing)

| Function | ESP32‑S3 GPIO |
|----------|----------------|
| Motor 0 PWM | 1 |
| Motor 0 DIR | 2 |
| Motor 0 ADC (pot) | 13 |
| Motor 1 PWM | 4 |
| Motor 1 DIR | 5 |
| Motor 1 ADC | 6 |
| Motor 2 PWM | 7 |
| Motor 2 DIR | 8 |
| Motor 2 ADC | 9 |
| Motor 3 PWM | 10 |
| Motor 3 DIR | 11 |
| Motor 3 ADC | 12 |
| UART2 RX (LiDAR) | 17 |
| UART2 TX (LiDAR) | 18 |
| UART1 RX (Radar) | 15 |
| UART1 TX (Radar) | 16 |
| I2C SDA (IMU/OLED) | 40 |
| I2C SCL (IMU/OLED) | 41 |

> ⚠️ Avoid strapping pins (GPIO 0, 3, 46, etc.) on ESP32‑S3.

---

## 6. Calibration & Testing Phases

1. **Suspended test** – verify each joint moves, potentiometer ADC read correct min/max.  
2. **PID tuning** – one leg at a time, hold position against external force.  
3. **UDP communication** – loopback with companion PC.  
4. **OpenClaw in simulation** – send joint targets, observe behaviour.  
5. **On‑floor crawling** – start with static gait (3 legs on ground).  
6. **IMU feedback** – tilt compensation.  
7. **LiDAR + SLAM** – build map while teleoperating.  
8. **Nav2 autonomous goal** – full autonomous walking.

---

## 7. Safety Features (Mandatory)

- Hardware E‑stop button (cuts 7.4V to SA8301)  
- mmWave human detection (LD2420) → emergency stop in autonomous mode  
- Watchdog timer in ESP32 (PWM=0 on lost connection)  
- Current limiting: monitor PWM duty cycle + optional shunt resistor  
- Low‑battery voltage cut‑off (<6.0V for 2S LiPo) → forced sit pose  
- Joint limit soft stops (from calibrated ADC min/max)

---

## 8. Expected Outcome

A low‑cost (≈$50 upgrade) autonomous quadruped robot that:
- Builds a 2D map of a room using LiDAR  
- Navigates to user‑specified goals via Nav2  
- Accepts voice or text commands through Telegram/WhatsApp (RosClaw)  
- Runs ROS2 with OpenClaw for stable gaits  
- Is fully open‑source (MIT license) and reproducible

---

## 9. Repository Structure (Suggested)

