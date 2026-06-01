# Toy Robot Dog Hardware Guide

This directory contains hardware documentation specific to transforming a toy robot dog into the ClawDog autonomous quadruped platform.

# HARDWARE.md - Toy Robot Dog Hardware Analysis

## 📋 Executive Summary

This document details the hardware analysis of the toy robot dog being converted into the **ClawDog** autonomous platform. Understanding the original hardware is critical for safe, non-destructive integration of new components.

> **Key Finding:** The original control board uses a **proprietary MCU** (likely JX3721), **not** an ESP32. This means we cannot simply reflash it—we must intercept control signals or replace the board entirely.

---

## 🔍 Original Hardware Identification

### Control Board
| Marking | Function | Notes |
|---------|----------|-------|
| `SDL-8008+2.4G+YW-RV3-ZXC` | Main control PCB | Likely custom design for this toy model |
| `SA8301` (×5) | H-bridge motor drivers | Controls DC motors via PWM + DIR |
| Large unmarked QFP chip | Main MCU | Likely JX3721 or similar proprietary controller |
| 2.4GHz module | RF receiver | Pairs with remote control |
| Various passives | Power regulation, filtering | LDOs, capacitors, resistors |

### Remote Control
| Marking | Function | Notes |
|---------|----------|-------|
| `SDL-8002-YW-2.4G-MIC-TX2-ZXC` | 2.4GHz transmitter | Sends commands to dog |
| AF256P2K13-45A4 | RF module | Standard 2.4GHz transceiver |
| Buttons + joystick | User input | Mapped to motor commands |

### Motors & Feedback
| Component | Specification | Connection |
|-----------|--------------|------------|
| DC Motor (×4) | 3-4.2V, ~0.5A each | 2× red wires to SA8301 outputs |
| Potentiometer (×4) | Linear taper, 3-terminal | Red: VCC, Blue: GND, White: SIGNAL |
| Joint mechanism | 1 DOF per leg (simplified) | Motor + potentiometer coupled mechanically |

### Power System (Original)
| Component | Specification | Limitation |
|-----------|--------------|------------|
| Battery | 1× 14500 LiPo (3.7V nominal) | ~1000mAh capacity |
| Runtime | ~5-7 minutes real-world | Insufficient for added electronics |
| Charging | USB or dedicated charger | No balancing circuit visible |

---

## 🔌 Signal Analysis

### Motor Control Protocol
```
MCU → SA8301 → Motor

Control signals per motor:
┌─────────────────────────────────┐
│ PWM Pin  │ Direction control    │
│          │ - Logic HIGH: Forward│
│          │ - Logic LOW: Reverse │
├─────────────────────────────────┤
│ DIR Pin  │ Speed control        │
│          │ - 5kHz PWM signal    │
│          │ - Duty cycle: 0-100% │
└─────────────────────────────────┘

SA8301 Pinout (typical):
┌────┬────────────────┐
│ 1  │ IN1 (PWM)      │
│ 2  │ IN2 (DIR)      │
│ 3  │ GND            │
│ 4  │ OUT1 → Motor A │
│ 5  │ OUT2 → Motor B │
│ 6  │ VCC (4.2-16V)  │
│ 7  │ EN (enable)    │
│ 8  │ GND            │
└────┴────────────────┘
```

### Position Feedback Loop
```
Potentiometer → ADC → MCU → Control Algorithm

Potentiometer wiring:
┌────┬─────────────────────┐
│ Red   │ VCC (3.3V or 5V) │
│ Blue  │ GND              │
│ White │ SIGNAL → ADC     │
└────┴─────────────────────┘

ADC characteristics (estimated):
- Resolution: 10-12 bit
- Reference: 3.3V or matching VCC
- Range: 0-180° or 0-360° mechanical rotation
```

## ⚡ Power Architecture (Original)

### Voltage Rails (Estimated)
```
[14500 LiPo: 3.7-4.2V]
        │
   ┌────┴────┐
   ▼         ▼
[Direct]  [LDO?]
   │         │
   ▼         ▼
[SA8301]  [MCU + Logic]
[VCC: 4.2V] [3.3V estimated]
   │
   ▼
[DC Motors]
[3.7-4.2V]
```

## 🔧 Modification Points

### Safe Integration Strategy: Parallel Signal Tap

```
Original Signal Path:
[MCU] ──PWM/DIR──► [SA8301] ──► [Motor]

Your Addition (Non-Destructive):
[MCU] ──PWM/DIR──┬──► [74HC125 Buffer] ──► [SA8301]
                 │         ▲
                 │         │ (Enable from ESP32)
                 │
[ESP32-S3] ──PWM/DIR──┘

Feedback Monitoring:
[Potentiometer] ──SIGNAL──┬──► [MCU ADC]
                          │
                          └──► [ESP32 ADC] (via voltage divider if needed)
```

**Last Updated**: 2026-05-21  
**Document Version**: 1.0  
**Project**: ClawDog - Autonomous Quadruped Conversion
