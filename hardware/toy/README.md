# Toy Robot Dog Hardware Guide

This directory contains hardware documentation specific to transforming a toy robot dog into the ClawDog autonomous quadruped platform.

## Overview

The ClawDog project converts an off-the-shelf toy robot dog into an intelligent autonomous quadruped by:
1. Replacing the original control electronics with an **ESP32-S3-N16R8** microcontroller
2. Installing advanced sensors (LiDAR, mmWave radar, IMU)
3. Integrating with ROS2 for autonomous navigation and control
4. Maintaining the original mechanical structure and leg actuators

## Directory Contents

- **[README.md](./README.md)** - This file
- **[BOM.md](./BOM.md)** - Complete bill of materials with part numbers and suppliers
- **[assembly.md](./assembly.md)** - Step-by-step assembly instructions
- **[modifications.md](./modifications.md)** - Physical modifications to the toy chassis
- **[wiring-guide.md](./wiring-guide.md)** - Detailed wiring diagrams and connections

## Quick Start

1. **Gather Components** → Review [BOM.md](./BOM.md)
2. **Prepare Chassis** → Follow [modifications.md](./modifications.md) for physical prep
3. **Assemble Electronics** → Use [assembly.md](./assembly.md) for step-by-step guidance
4. **Wire Everything** → Reference [wiring-guide.md](./wiring-guide.md)
5. **Test & Calibrate** → See hardware testing in [assembly.md](./assembly.md)

## Key Specifications

| Aspect | Details |
|--------|---------|
| **Base Platform** | Toy robot dog (quadruped, ~500g) |
| **Controller** | ESP32-S3-N16R8 (16MB flash, 8MB PSRAM, dual-core 240MHz) |
| **Motors** | 4× DC motors with potentiometer feedback (original toy motors) |
| **Motor Drivers** | 5× SA8301 dual H-bridge (1 spare) |
| **Primary Sensor** | WitMotion D6 LiDAR (360°, 0.1-12m, 10Hz) |
| **Secondary Sensor** | LD2420 mmWave radar (24GHz, ±60° FOV) |
| **IMU** | BNO055 9-DOF (accel, gyro, mag) |
| **Power** | 2S LiPo battery (7.4V, 2200-3000mAh) |
| **Estimated Weight** | ~750g (final assembly) |
| **Runtime** | 30-40 minutes typical use |

## Critical Considerations

### Weight & Balance
- Total estimated weight: **~750g** (+50% from original)
- Place heavy components (battery, LiDAR) low and centered
- Monitor balance during assembly—leg servo torque is limited

### Power Management
- **Peak current**: 5-6A (motor stall + LiDAR spin-up)
- **Typical current**: ~2.7A (balanced operation)
- Battery runtime: 30-40 min typical, 15-20 min conservative
- Fuse all motor power lines (5A recommended)

### GPIO Constraints (ESP32-S3-N16R8)
- **Reserved for PSRAM**: GPIO 26-32 (unavailable)
- **Reserved for USB**: GPIO 19-20, 43-44
- **Strapping pins**: GPIO 0, 3, 45, 46 (use cautiously)
- **Available for I/O**: GPIO 1-2, 4-25, 33-42
- See [pinout reference](../overview.md#esp32-s3-pin-restrictions-n16r8-with-octal-psram)

### Leg Motor Feedback
- Original toys use **potentiometer feedback** on leg servos (3-wire connectors)
- Potentiometer signals connect to ESP32 ADC pins
- **Must verify connector polarity before connecting** (use multimeter)
- Typical signal: VCC (red), Signal (white), GND (black)

## Safety Notes

⚠️ **Before first power-on:**
1. Verify all connections with multimeter
2. Test power rails under no-load condition
3. Check for shorts between VCC and GND
4. Confirm motor connectors are correctly installed
5. Keep fingers clear of spinning LiDAR and motor shafts

## Testing Procedures

Refer to [assembly.md](./assembly.md) for:
- Power-on verification
- Communication bus testing
- Motor response and control
- Sensor calibration
- Full system integration test

## Troubleshooting

Common issues and solutions:

| Issue | Likely Cause | Solution |
|-------|--------------|----------|
| No boot response | Incorrect USB connection or driver | Check CP2104 drivers; try different USB cable |
| Motors don't respond | Wrong pin assignments or pot polarity | Verify [wiring-guide.md](./wiring-guide.md); reverse pot connections if needed |
| LiDAR not detected | UART baud rate mismatch | Confirm UART2 @ 921600 baud in firmware |
| Potentiometer readings unstable | Motor noise coupling | Add ferrite beads on motor power; increase ADC averaging |
| Overheating SA8301 drivers | Sustained high duty cycle | Add heatsinks; reduce duty cycle limits; improve airflow |

## Integration with Main Project

This toy hardware serves as the platform for:
- **ROS2 integration** (see `/ros2` directory)
- **Firmware development** (see `/firmware` directory)
- **Autonomous navigation** via LiDAR/radar data fusion
- **OpenClaw framework** integration (future)

## References

- Main hardware overview: [../overview.md](../overview.md)
- Pinout reference: [../pinout.md](../pinout.md)
- ESP32-S3 datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
- Hardware design guidelines: https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/

## Contributing

To update toy hardware documentation:
1. Make changes to relevant `.md` files
2. Test procedures on actual hardware
3. Update BOM with accurate part numbers and availability
4. Submit PR with clear description of changes

---

**Last Updated**: [Auto-populated by CI]  
**Status**: Active Development  
**Maintainer**: ClawDog Hardware Team
