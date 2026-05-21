# Hardware Overview

## System Components

### 1. Main Controller: ESP32-S3-N16R8

| Specification | Value |
|---------------|-------|
| Model | ESP32-S3-N16R8 |
| Flash | 16MB (Quad SPI) |
| PSRAM | 8MB (Octal SPI) |
| CPU | Dual-core Xtensa LX7 @ 240MHz |
| WiFi | 802.11 b/g/n |
| Bluetooth | BLE 5.0 |
| ADC | 2× 12-bit SAR ADC, 20 channels |
| UART | 3× UART |
| I2C | 2× I2C |
| SPI | 4× SPI |
| PWM | LEDC (up to 8 channels) |

**Critical**: The N16R8 variant uses **Octal PSRAM** which reserves GPIO 26-32 for SPI/PSRAM interface. These pins are **unavailable** for general purpose I/O.

### 2. Motor Drivers: SA8301 (×5)

| Specification | Value |
|---------------|-------|
| Type | Dual H-Bridge |
| Voltage Range | 2.5V - 16V |
| Max Current | 1.5A continuous, 2.5A peak per channel |
| Logic Level | 3.3V - 5V compatible |
| PWM Frequency | Up to 100kHz |

Usage:
- 4 channels for leg motors (1 DOF per leg)
- 1 spare channel (unused)

### 3. Sensors

#### WitMotion D6 LiDAR
| Specification | Value |
|---------------|-------|
| Type | dTOF (direct Time of Flight) |
| Range | 0.1m - 12m |
| Angle | 360° |
| Resolution | 1° |
| Frequency | 10Hz (10 rotations/sec) |
| Interface | UART @ 921600 baud |
| Voltage | 5V |
| Current | ~800mA typical, 1.5A peak |

#### LD2420 mmWave Radar
| Specification | Value |
|---------------|-------|
| Type | 24GHz FMCW mmWave |
| Detection Range | 0.5m - 6m (configurable) |
| Detection Angle | ±60° horizontal, ±30° vertical |
| Update Rate | 10Hz |
| Interface | UART @ 115200 baud |
| Voltage | 5V or 3.3V (verify module variant) |
| Current | ~50mA |

#### BNO055 IMU
| Specification | Value |
|---------------|-------|
| Type | 9-DOF (Accelerometer + Gyroscope + Magnetometer) |
| Interface | I2C @ 400kHz |
| Address | 0x28 (default) or 0x29 |
| Voltage | 3.3V |
| Current | ~12mA |

#### OLED Display (0.96" 128×64)
| Specification | Value |
|---------------|-------|
| Type | Monochrome OLED |
| Resolution | 128×64 pixels |
| Driver | SSD1306 (confirmed) |
| Interface | I2C @ 400kHz |
| Address | 0x3C (common) or 0x3D |
| Voltage | 3.3V - 5V |
| Current | ~15mA (display on) |

#### Potentiometers (×4)
| Specification | Value |
|---------------|-------|
| Type | Rotary/Linear potentiometer (3-wire) |
| Resistance | 10kΩ typical |
| Voltage | 3.3V reference |
| Resolution | 12-bit ADC (4096 steps) |

### 4. Power System

#### Battery
| Specification | Value |
|---------------|-------|
| Type | 2S LiPo |
| Voltage | 7.4V nominal, 8.4V max, 6.0V min |
| Capacity | 2200-3000mAh |
| Discharge Rate | 25C+ |
| Connector | XT60 or JST (verify) |

#### Power Regulators

**MP1584 Buck Converter (5V)**
- Input: 7.4V (battery)
- Output: 5V / 3A
- Efficiency: ~90%
- Used for: LiDAR

**AMS1117-3.3 LDO (3.3V)**
- Input: 7.4V (battery)
- Output: 3.3V / 1A
- Efficiency: ~45% (linear, produces heat)
- Used for: ESP32-S3, BNO055, LD2420 (if 3.3V variant)

## ESP32-S3 Pin Restrictions (N16R8 with Octal PSRAM)

### Reserved Pins (DO NOT USE)

| GPIO | Function | Notes |
|------|----------|-------|
| 26 | SPIHD | Octal PSRAM data |
| 27 | SPIWP | Octal PSRAM data |
| 28 | SPICS0 | Octal PSRAM chip select |
| 29 | SPICLK | Octal PSRAM clock |
| 30 | SPIQ | Octal PSRAM data |
| 31 | SPID | Octal PSRAM data |
| 32 | SPIQ/SPID | Octal PSRAM data (depending on package) |

### Strapping Pins (Avoid or Use with Caution)

| GPIO | Function | Risk |
|------|----------|------|
| 0 | Boot mode | Pulled low = download mode, high = normal boot |
| 3 | JTAG | Used for debugging |
| 45 | VDD_SPI voltage | Sets flash voltage (3.3V vs 1.8V) |
| 46 | ROM log print | Affects boot messages |

### USB Pins (Reserved)

| GPIO | Function |
|------|----------|
| 19 | USB D- |
| 20 | USB D+ |
| 43 | USB D- (alternate) |
| 44 | USB D+ (alternate) |

### Available Pins Summary

**Safe to use** (no restrictions):
- GPIO 1-2, 4-9, 10-14, 15-18, 21-25, 33-42

**Use with caution**:
- GPIO 0 (strapping)
- GPIO 3 (strapping/JTAG)
- GPIO 45-46 (strapping)

**Reserved**:
- GPIO 26-32 (PSRAM)
- GPIO 19-20, 43-44 (USB)

## Current Pin Assignment Review

Current assignments vs restrictions:

| Function | GPIO | Status |
|----------|------|--------|
| Motor 0 PWM | 1 | ✅ Safe |
| Motor 0 DIR | 2 | ✅ Safe |
| Motor 0 ADC | 3 | ⚠️ Strapping pin (JTAG) - **Consider moving** |
| Motor 1 PWM | 4 | ✅ Safe |
| Motor 1 DIR | 5 | ✅ Safe |
| Motor 1 ADC | 6 | ✅ Safe |
| Motor 2 PWM | 7 | ✅ Safe |
| Motor 2 DIR | 8 | ✅ Safe |
| Motor 2 ADC | 9 | ✅ Safe |
| Motor 3 PWM | 10 | ✅ Safe |
| Motor 3 DIR | 11 | ✅ Safe |
| Motor 3 ADC | 12 | ✅ Safe |
| UART1 RX | 15 | ✅ Safe |
| UART1 TX | 16 | ✅ Safe |
| UART2 RX | 17 | ✅ Safe |
| UART2 TX | 18 | ✅ Safe |
| I2C SDA | 40 | ✅ Safe |
| I2C SCL | 41 | ✅ Safe |

### Recommended Changes

**GPIO 3 (Motor 0 ADC) is a strapping pin!** Move to a safe GPIO.

**Recommended alternatives for Motor 0 ADC**:
- GPIO 13
- GPIO 14
- GPIO 21 (if not using USB)

Updated pin assignment:

| Function | GPIO | Notes |
|----------|------|-------|
| Motor 0 PWM | 1 | To SA8301 IN1 |
| Motor 0 DIR | 2 | To SA8301 IN2 |
| **Motor 0 ADC** | **13** | **Potentiometer (moved from GPIO 3)** |
| Motor 1 PWM | 4 | To SA8301 IN1 |
| Motor 1 DIR | 5 | To SA8301 IN2 |
| Motor 1 ADC | 6 | Potentiometer |
| Motor 2 PWM | 7 | To SA8301 IN1 |
| Motor 2 DIR | 8 | To SA8301 IN2 |
| Motor 2 ADC | 9 | Potentiometer |
| Motor 3 PWM | 10 | To SA8301 IN1 |
| Motor 3 DIR | 11 | To SA8301 IN2 |
| Motor 3 ADC | 12 | Potentiometer |
| UART1 RX | 15 | mmWave radar LD2420 |
| UART1 TX | 16 | mmWave radar LD2420 |
| UART2 RX | 17 | LiDAR |
| UART2 TX | 18 | LiDAR |
| I2C SDA | 40 | BNO055 IMU, OLED Display |
| I2C SCL | 41 | BNO055 IMU, OLED Display |

## Physical Dimensions

### Robot Dog (Toy Base)
- Weight: ~500g (without modifications)
- Length: ~250mm
- Width: ~150mm
- Height: ~180mm

### Modified Weight Budget
| Component | Weight |
|-----------|--------|
| Original toy | ~500g |
| ESP32-S3 module | ~5g |
| LiDAR | ~80g |
| mmWave radar | ~10g |
| IMU | ~5g |
| Battery (2200mAh) | ~120g |
| Power converters | ~30g |
| **Total estimated** | **~750g** |

## Assembly Notes

1. **Weight Distribution**: Place heavy components (battery, LiDAR) low and centered for stability
2. **Cable Management**: Use zip ties and cable guides to prevent leg snagging
3. **Vibration Isolation**: Mount IMU with foam tape or soft mounting
4. **Heat Dissipation**: Ensure SA8301 drivers have airflow; add heatsinks if needed
5. **Waterproofing**: Consider conformal coating for outdoor use
6. **Accessibility**: Design for easy battery swap and USB access

## Testing Checklist

### Power-On Test
- [ ] Battery voltage reading correct (7.4V nominal)
- [ ] 5V rail stable under load (±5%)
- [ ] 3.3V rail stable under load (±3%)
- [ ] No smoke or unusual heat

### Communication Test
- [ ] ESP32 boots successfully
- [ ] WiFi connects
- [ ] USB serial available for debugging
- [ ] I2C scan detects BNO055
- [ ] UART1 receives LD2420 data
- [ ] UART2 receives LiDAR data

### Motor Test
- [ ] Each motor responds to PWM
- [ ] Direction control works
- [ ] ADC reads potentiometer
- [ ] No excessive heat or noise

## References

- ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
- ESP32-S3 Technical Reference: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/
- ESP32-S3 Hardware Design Guidelines: https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/
- SA8301 Datasheet: Search "SA8301 H-bridge datasheet"
- BNO055 Datasheet: https://www.bosch-sensortec.com/products/smart-sensors/bno055/
- LD2420 Datasheet: Search "LD2420 mmWave radar datasheet"
