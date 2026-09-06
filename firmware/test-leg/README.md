# Single Leg Test Rig - Wiring Guide

**Goal:** Make one leg move slowly and safely. Perfect first project to learn ESP32 motor control.

## What You Need

| Item | Source | Notes |
|------|--------|-------|
| ESP32-S3 dev board | Your parts | Any ESP32-S3 with USB-C |
| Toy dog mainboard | From your toy | You already cut the original chip contacts |
| One leg assembly | From your toy | Motor + potentiometer + gearbox |
| USB-C cable | Any | For power and programming |
| 3 jumper wires (F-M or M-M) | Electronics shop | To connect ESP32 to mainboard |
| Multimeter | Recommended | To verify voltages before connecting |
| Battery or power supply | 7.4V-8.4V | 2S LiPo or bench power supply |

## How It Connects

You already cut the original chip from the toy mainboard. Now the SA8301 motor driver chips on that board are "free" — they just need new control signals from the ESP32.

```
ESP32-S3                    Toy Mainboard
┌─────────┐                 ┌─────────────┐
│         │                 │             │
│ GPIO 1  ├────────────────→│ SA8301 IN1  │←── Motor power control
│  (PWM)  │    Wire 1       │   (PWM)     │
│         │                 │             │
│ GPIO 2  ├────────────────→│ SA8301 IN2  │←── Motor direction control
│  (DIR)  │    Wire 2       │   (DIR)     │
│         │                 │             │
│ GPIO 13 ├←───────────────┤ Pot Signal  │←── Leg angle sensor
│  (ADC)  │    Wire 3       │  (White?)   │
│         │                 │             │
│   GND   ├────────────────→│    GND      │←── IMPORTANT: Common ground!
│         │    Wire 4       │             │
│   3.3V  ├────────────────→│ Pot VCC     │←── Only if mainboard doesn't power it
│         │    Wire 5       │  (Red?)     │
└─────────┘                 └─────────────┘
                                   │
                                   └──→ Motor (already connected on mainboard)
```

## Step-by-Step Wiring

### Step 0: Safety Check
- **Do NOT connect motors yet** (or prop the leg so it can't hit anything)
- Have your USB cable ready to unplug quickly
- Work on a non-conductive surface (wood table, not metal)

### Step 1: Find the Right Pins on Your Mainboard

Since you cut the original chip, look for traces going to the SA8301 chips. You need:
- **IN1 and IN2** of one SA8301 → these will connect to ESP32 GPIO 1 and 2
- **Potentiometer signal** → connects to ESP32 GPIO 13

Use a multimeter in continuity mode to find which pads on the board connect to:
- SA8301 pin 2 (IN1) and pin 3 (IN2) — check the SA8301 datasheet
- The potentiometer wiper (middle pin of the 3-pin pot connector)

### Step 2: Connect Ground (MOST IMPORTANT)

**You MUST connect ESP32 GND to mainboard GND.** Without this, the motor control signals have no reference and won't work.

1. Find a GND pad on your mainboard (usually near the battery connector or regulator)
2. Connect ESP32 GND → Mainboard GND with a jumper wire

### Step 3: Connect PWM and DIR

1. Connect ESP32 GPIO 1 → SA8301 IN1 (or the pad you found)
2. Connect ESP32 GPIO 2 → SA8301 IN2 (or the pad you found)

### Step 4: Connect Potentiometer

The leg has a potentiometer that tells you what angle it's at.

1. Connect ESP32 GPIO 13 → Potentiometer signal (usually white or yellow wire)
2. If the mainboard doesn't power the pot: Connect ESP32 3.3V → Pot VCC (red wire), and ESP32 GND → Pot GND (black wire)

**Verify first:** With a multimeter, check that the potentiometer reads 0V to ~3.3V when you move the leg by hand. If it reads 0V to 7V, DO NOT connect to ESP32 — that will damage it!

### Step 5: Power On

1. Connect USB-C to ESP32 (for programming and power)
2. Connect battery to mainboard (for motor power)
3. Open Serial Monitor at 115200 baud

## Testing

### First Test: ADC Only (No Motor Movement)

Before running the motor, verify the potentiometer works:

1. Upload the code
2. Move the leg by hand
3. Watch Serial Monitor — ADC values should change from ~500 to ~3500
4. If values don't change, check potentiometer wiring

### Second Test: Very Slow Movement

1. Prop the leg so it can't hit anything (or remove it from the body)
2. Have your hand on the USB cable, ready to unplug
3. The code waits 5 seconds before any movement
4. The leg should move VERY slowly — if it's fast, something is wrong, unplug immediately!

### Emergency Commands

While running, send these via Serial Monitor:
- `s` — Stop motor immediately
- `r` — Resume sweep

## Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| Motor doesn't move | No common GND | Connect ESP32 GND to mainboard GND |
| Motor spins too fast | PWM signal not working | Check GPIO 1 connection, verify with multimeter |
| ADC values don't change | Wrong pot pin | Use multimeter to find the wiper (middle) pin |
| ADC values jump around | Motor noise | Normal — code has filter, check grounding |
| ESP32 resets when motor runs | Power issue | Motor draws too much current, add capacitors near SA8301 |
| Leg moves in wrong direction | DIR pin inverted | Swap GPIO 1 and 2, or change `forward` boolean in code |

## What Success Looks Like

When it works, you should see in Serial Monitor:

```
========================================
  ClawDog - Single Leg Test Rig
========================================
SAFETY: 5-second pause before movement.
Commands: 's' = stop, 'r' = resume sweep
========================================

[INIT] Starting sweep...
Sweep UP | ADC: 1245 | Duty: 50
Sweep UP | ADC: 1267 | Duty: 100
...
[SWITCH] Sweeping down...
Sweep DOWN | ADC: 1301 | Duty: 100
...
```

The leg should move slowly back and forth, and the ADC value should smoothly track the position.

## micro-ROS Setup (Optional - Advanced)

This firmware includes micro-ROS support for integration with ROS2. If you want to control the robot via ROS2 (e.g., teleop_twist_keyboard), follow these steps:

### Prerequisites

- ROS2 Humble or Jazzy installed on a computer
- micro-ROS agent installed

### 1. Install micro-ROS Agent

```bash
# On your ROS2 computer
sudo apt install ros-$ROS_DISTRO-micro-ros-agent
```

### 2. Configure WiFi and Agent IP

Edit `platformio.ini` and set your WiFi credentials and agent IP:

```ini
build_flags = 
    -D CORE_DEBUG_LEVEL=3
    -D BOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
    -D WIFI_SSID=\"YOUR_WIFI_SSID\"
    -D WIFI_PASS=\"YOUR_WIFI_PASSWORD\"
    -D AGENT_IP=\"192.168.1.100\"
    -D AGENT_PORT=\"8888\"
```

Or set them via environment variables before building:
```bash
export WIFI_SSID="your_ssid"
export WIFI_PASS="your_password"
export AGENT_IP="192.168.1.100"
pio run
```

### 3. Start the micro-ROS Agent

On your ROS2 computer:
```bash
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
```

### 4. Upload Firmware

```bash
cd firmware
pio run --target upload
```

### 5. Verify Connection

Open Serial Monitor:
```bash
pio device monitor --baud 115200
```

You should see:
```
[WiFi] Connected! IP: 192.168.1.xxx
[INIT] Setup complete, waiting for micro-ROS agent...
[AGENT] Connected!
```

### 6. Test with ROS2

List topics:
```bash
ros2 topic list
```

You should see:
- `/joint_states` - Current leg positions
- `/battery_voltage` - Battery voltage
- `/heartbeat` - Connection status
- `/cmd_vel` - Velocity commands (subscribe)

Send test command:
```bash
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.5}, angular: {z: 0.0}}"
```

### 7. Teleop Control

Install teleop:
```bash
sudo apt install ros-$ROS_DISTRO-teleop-twist-keyboard
```

Run:
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

Use keys to control the robot. The ESP32 will receive Twist messages and convert them to motor commands.

### Troubleshooting micro-ROS

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| `[AGENT] Connection failed` | Agent not running | Start agent: `ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888` |
| `[AGENT] Connection lost` | Network issue | Check WiFi, firewall, IP address |
| `WiFi connection failed` | Wrong credentials | Check WIFI_SSID/WIFI_PASS in platformio.ini |
| Topics not appearing | Agent not connected | Wait for `[AGENT] Connected!` message |
| Motors don't respond to /cmd_vel | Emergency stop active | Check battery voltage, reset ESP32 |

## Next Steps

Once one leg works:
1. Calibrate ADC min/max (move leg to physical limits, record values)
2. Add PID control to hold a target position
3. Repeat for all 4 legs
4. Try micro-ROS teleop control
5. Add IMU and try balancing

## Pin Reference

From `docs/hardware/pinout.md`:

| Function | GPIO | Your Wire Color |
|----------|------|-----------------|
| Motor PWM | 1 | ? |
| Motor DIR | 2 | ? |
| Motor ADC | 13 | ? |
| GND | GND | Black |
| 3.3V | 3.3V | Red (if needed) |

**Fill in the "Your Wire Color" column as you wire it — this will help when you add the other 3 legs.**
