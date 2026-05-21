# Hardware Pinout Reference

## ESP32-S3-H16R8 Pin Assignments

### Motor Control

| Function | GPIO | Direction | Notes |
|----------|------|-----------|-------|
| Motor 0 PWM | 1 | Output | To SA8301 IN1 |
| Motor 0 DIR | 2 | Output | To SA8301 IN2 |
| Motor 0 ADC | 13 | Input | Potentiometer signal |
| Motor 1 PWM | 4 | Output | To SA8301 IN1 |
| Motor 1 DIR | 5 | Output | To SA8301 IN2 |
| Motor 1 ADC | 6 | Input | Potentiometer signal |
| Motor 2 PWM | 7 | Output | To SA8301 IN1 |
| Motor 2 DIR | 8 | Output | To SA8301 IN2 |
| Motor 2 ADC | 9 | Input | Potentiometer signal |
| Motor 3 PWM | 10 | Output | To SA8301 IN1 |
| Motor 3 DIR | 11 | Output | To SA8301 IN2 |
| Motor 3 ADC | 12 | Input | Potentiometer signal |

**PWM Configuration**:
- Frequency: 1kHz (start), adjust based on motor response
- Resolution: 10-bit (0-1023)
- Max duty cycle: 80% (819 for 10-bit) to prevent overheating

**ADC Configuration**:
- Resolution: 12-bit (0-4095)
- Attenuation: 11dB (full 0-3.3V range)
- Sampling: 5-10 sample moving average for noise reduction

### Communication

| Function | GPIO | Direction | Notes |
|----------|------|-----------|-------|
| UART2 RX | 17 | Input | LiDAR data (921600 baud) |
| UART2 TX | 18 | Output | LiDAR config (rarely used) |
| UART1 RX | 15 | Input | mmWave radar LD2420 (115200 baud) |
| UART1 TX | 16 | Output | mmWave radar LD2420 config |
| I2C SDA | 40 | Bidirectional | BNO055 IMU, OLED Display |
| I2C SCL | 41 | Output | BNO055 IMU, OLED Display |

**UART2 Configuration** (LiDAR):
- Baud rate: 921600
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None

**UART1 Configuration** (mmWave Radar LD2420):
- Baud rate: 115200
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None
- Protocol: Custom binary frame protocol

**I2C Configuration**:
- Frequency: 400kHz (Fast Mode)
- Pull-ups: 4.7kΩ external (required)
- BNO055 Address: 0x28 (default) or 0x29
- OLED Address: 0x3C (common) or 0x3D (alternate)

### Reserved / Special Pins

| GPIO | Status | Reason |
|------|--------|--------|
| 0 | ⚠️ Strapping pin | Boot mode selection - avoid using |
| 3 | ⚠️ Strapping pin | JTAG signal - avoid using |
| 46 | ⚠️ Strapping pin | ROM log print - avoid using |
| 19-21 | Reserved | USB/JTAG - don't use if debugging |
| 43-44 | USB | USB D-/D+ - reserved |

## SA8301 Motor Driver Connections

Each SA8301 controls one DC motor:

| SA8301 Pin | Connection | Description |
|------------|------------|-------------|
| VCC | 7.4V (battery) | Motor power supply |
| GND | GND (battery) | Motor ground |
| IN1 | ESP32 PWM GPIO | Speed/direction control |
| IN2 | ESP32 DIR GPIO | Direction control |
| OUT1 | Motor terminal 1 | Motor connection |
| OUT2 | Motor terminal 2 | Motor connection |

**Logic Level**: SA8301 inputs are 3.3V compatible (verify with specific datasheet).

## Power Distribution

### Power Tree
```
2S LiPo 7.4V (8.4V max, 6.0V min)
    │
    ├───→ SA8301 VCC pins (direct, fused)
    │     Fuse: 5A fast-blow per driver
    │
    ├───→ MP1584 Buck Converter
    │     Input: 7.4V
    │     Output: 5V/3A
    │     └───→ WitMotion D6 LiDAR (5V, ~1.5A peak)
    │
    └───→ AMS1117-3.3 LDO
          Input: 7.4V
          Output: 3.3V/1A
          └───→ ESP32-S3 VCC (3.3V, ~500mA)
          └───→ BNO055 VCC (3.3V, ~50mA)
```

### Current Budget

| Component | Typical | Peak | Notes |
|-----------|---------|------|-------|
| 4× Motors | 1.5A | 3.0A | Stall current ~1A each |
| ESP32-S3 | 300mA | 500mA | With WiFi active |
| LiDAR | 800mA | 1500mA | During spin-up |
| BNO055 | 12mA | 20mA | Normal operation |
| LD2420 | 50mA | 80mA | Normal operation |
| OLED | 15mA | 25mA | Display on, all pixels |
| **Total** | **~2.7A** | **~5.1A** | Plan for 6A peak |

**Battery Selection**: 2S LiPo 2200mAh+ with 25C+ discharge rating
**Runtime Estimate**: 30-40 minutes typical, 15-20 minutes conservative

## Wiring Best Practices

1. **Grounding**: Single-point ground connection between power and logic grounds
2. **Decoupling**: 100nF ceramic + 10µF electrolytic per SA8301 VCC pin
3. **Motor Noise**: Ferrite beads on motor power lines
4. **ADC Quality**: Separate analog ground plane, shielded cable for potentiometer signals
5. **LiDAR Power**: Dedicated buck converter, no shared loads
6. **USB Debugging**: Keep USB disconnected during motor testing to prevent ground loops

## Connector Pinouts

### Leg Connector (Original Toy)
Typical 3-wire connector per leg:
- **Red**: VCC (potentiometer power, 3.3V)
- **White**: Signal (potentiometer wiper → ESP32 ADC)
- **Black**: GND (potentiometer ground)

**⚠️ Verify with multimeter before connecting to ESP32**

### LiDAR Connector (WitMotion D6)
Typical 4-pin JST or similar:
- **Red**: 5V power
- **Black**: GND
- **Yellow**: TX (LiDAR → ESP32 RX)
- **Green**: RX (ESP32 TX → LiDAR)

### OLED Display Connector (0.96" 128x64 I2C)
Typical 4-pin header:
- **VCC**: 3.3V or 5V (verify module - most accept 3.3V-5V)
- **GND**: Ground
- **SCL**: I2C clock (shared with BNO055)
- **SDA**: I2C data (shared with BNO055)

**OLED Configuration**:
- Resolution: 128×64 pixels
- Driver: SSD1306
- Interface: I2C @ 400kHz
- Address: 0x3C (common) or 0x3D
- Update rate: 30Hz recommended (don't block control loop)

**Display Content** (planned):
- Battery voltage
- WiFi status
- Current gait mode
- Joint angles (live)
- Error/safety messages

### mmWave Radar Connector (LD2420)
Typical 4-pin JST or similar:
- **Red**: 5V power (or 3.3V - verify with module variant)
- **Black**: GND
- **Yellow**: TX (Radar → ESP32 RX)
- **Green**: RX (ESP32 TX → Radar)

**LD2420 Configuration**:
- Detection range: 0.5m - 6m (configurable)
- Detection angle: ±60° horizontal, ±30° vertical
- Update rate: 10Hz default
- Output format: Target presence + distance + angle (when available)

**Power Note**: Some LD2420 variants require 5V, others 3.3V. **Verify before connecting.** If 5V variant, power from MP1584 5V rail. If 3.3V variant, power from AMS1117-3.3 rail.

## Calibration Procedure

1. **ADC Range Discovery**:
   ```cpp
   // Pseudo-code for calibration
   for each joint:
     move_to_physical_minimum()
     adc_min[joint] = read_adc_average(100 samples)
     
     move_to_physical_maximum()
     adc_max[joint] = read_adc_average(100 samples)
     
     // Store in non-volatile memory (EEPROM/flash)
   ```

2. **Angle Mapping**:
   ```cpp
   float adc_to_angle(uint16_t adc_value, uint16_t adc_min, uint16_t adc_max, float angle_min, float angle_max) {
     float t = (float)(adc_value - adc_min) / (float)(adc_max - adc_min);
     return angle_min + t * (angle_max - angle_min);
   }
   ```

3. **Safety Limits**:
   - Software limits: 5% margin inside physical limits
   - Emergency stop if ADC reads outside [min-10%, max+10%]
   - Log all limit events for debugging

## Known Hardware Issues

1. **GPIO 10/11 Conflict**: Original plan used GPIO 10/11 for I2C, but these may conflict with strapping or flash functions. **Use GPIO 40/41 instead**.

2. **Potentiometer Dead Zones**: Some pots may have non-linear regions at extremes. Test full range and add software deadband if needed.

3. **Motor Noise on ADC**: When motors run, ADC readings may fluctuate. Solutions:
   - Hardware: Separate AGND and DGND, add capacitors
   - Software: Increase moving average samples, read ADC when motor PWM is off

4. **SA8301 Heat**: Can get warm at high duty cycles. Add heatsink if sustained operation >60% duty cycle.

## References

- ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
- ESP32-S3 Technical Reference: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/
- SA8301 Datasheet: Search "SA8301 H-bridge datasheet"
- BNO055 Datasheet: https://www.bosch-sensortec.com/products/smart-sensors/bno055/
- WitMotion D6 Protocol: Contact WitMotion or reverse engineer from SDK
