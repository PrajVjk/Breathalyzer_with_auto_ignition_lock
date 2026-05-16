# Breathalyzer with Auto-Ignition Lock

An embedded systems project built on the **PIC18F4331** microcontroller that measures a user's Blood Alcohol Content (BAC) via an MQ-3 gas sensor and controls a servo-actuated ignition lock based on the result. If BAC is below the legal limit (0.08%), the servo rotates to simulate an ignition unlock. Otherwise, the system stays locked and the result is displayed on an LCD.

---

## Features

- **Real-time BAC measurement** using the MQ-3 alcohol gas sensor
- **Interrupt-driven flow** — a dip switch triggers `INT0` to initiate a reading without polling
- **Averaged ADC sampling** (20 samples) for stable, noise-resilient readings
- **Integer-only BAC conversion** using a precomputed lookup table with linear interpolation — no floating-point library needed (keeps flash usage within the PIC18F4331's 8K-word limit)
- **1602A OLED display** driven over a bit-banged serial interface (no I2C hardware peripheral required)
- **Servo motor control** via software bit-bang PWM at 50 Hz, sweeping 0°→180°→0° on pass
- **LED indicator** signals when the system is actively sampling
- **Safe/locked decision** compared against the 0.08% BAC legal threshold

---

## Hardware

| Component | Part | Connection |
|---|---|---|
| Microcontroller | PIC18F4331 | — |
| Alcohol Sensor | MQ-3 | RA4 / AN2 (analog input) |
| Display | 1602A OLED (serial) | RD0 (CS), RD2 (SDI), RD3 (SCL) |
| Servo Motor | SG-5010 | RB1 (bit-bang PWM) |
| Trigger Switch | DIP switch | RB0 / INT0 (external interrupt) |
| Status LED | Generic LED | RD1 (digital output) |

**Power:** 5V supply. Oscillator: 8 MHz internal RC (IRCF = 111).

---

## Software Architecture

The project is organized into four modules:

### `main.c` — System entry and control flow
Initializes all peripherals, displays a startup prompt, then loops waiting for the `g_startReading` flag set by the ISR. When triggered, it disables `INT0`, runs the full breathalyzer sequence, then re-enables the interrupt.

### `adc.c / adc.h` — MQ-3 sensor interface
- Configures the 10-bit ADC on AN2
- Provides `ADC_ReadAverage(n)` for multi-sample averaging
- `ADC_ToBacThou()` converts raw ADC values to BAC (in thousandths, e.g. `80` = 0.080%) using a 10-point lookup table derived from the MQ-3 sensitivity curve:
  ```
  Rs = RL × (Vc − Vout) / Vout
  ratio = Rs / Ro
  mg/L = 0.4 × ratio^(−1.43)
  BAC = mg/L × 0.21   (1:2100 breath-to-blood ratio)
  ```
  with RL = 10 kΩ, Ro = 3.3 kΩ, Vc = 5V.

### `lcd.c / lcd.h` — 1602A OLED display driver
Implements a bit-banged serial protocol (10 clock pulses per byte: RS + RW + D7..D0). Provides `LCD_Print()`, `LCD_SetCursor()`, `LCD_PrintBAC()`, and `LCD_DisplayResult()`. BAC is formatted manually (no `sprintf`) to avoid pulling in the standard library.

### `pwm.c / pwm.h` — Servo motor control
Uses software `__delay_us()` pulses instead of the hardware PCPWM module (which would conflict with INT0 on RB0). Provides `PWM_RotateCycle()`, which sweeps the servo 0°→180°→0° in discrete 45° steps to simulate ignition unlock.

---

## System Flow

```
Power On
   │
   ▼
System_Init() → ADC, LCD, PWM, INT0 configured
   │
   ▼
LCD: "Breathalyzer System Ready" → "Press button to start test"
   │
   ▼ (user presses dip switch)
INT0 ISR fires → LED ON, g_startReading = 1
   │
   ▼
INT0 disabled → ProcessBreathalyzer()
   │
   ├── LCD: "Blow now..." (500 ms delay)
   ├── LCD: "Reading..."  (3000 ms blow window)
   ├── ADC_ReadAverage(20) → raw value
   ├── ADC_ToBacThou()     → BAC in thousandths
   ├── LCD_DisplayResult() → "Your BAC is X.XXX" + "Safe" / "!! Too High !!"
   │
   ├── [BAC < 0.08] → PWM_RotateCycle() (servo unlocks)
   └── [BAC ≥ 0.08] → servo stays locked
   │
   ▼
LED OFF → INT0 re-enabled → idle ("Press button to start test")
```

---

## Build & Flash

**Toolchain:** MPLAB X IDE with XC8 Compiler  
**Programmer/Debugger:** PICkit 5

1. Clone or copy the project files into a new MPLAB X project targeting the **PIC18F4331**.
2. Add all `.c` and `.h` files to the project.
3. Build with XC8 (Free mode is sufficient — flash usage stays well within the device's 8K-word limit).
4. Flash using PICkit 5 via ICSP.

> **Note:** The `_XTAL_FREQ` macro is set to `8000000UL` in `config.h`. Ensure the configuration bits match (`OSC = IRCIO`).

---

## Calibration

The ADC-to-BAC lookup table in `adc.c` is derived from the MQ-3 datasheet characteristic curve and theoretical sensor parameters. For accurate readings in a real deployment, calibrate with known alcohol concentration samples and adjust the `adc_points[]` and `bac_points[]` arrays accordingly.

---

## Project Structure

```
.
├── main.c                    # Main program, ISR, system init
├── config.h                  # Config bits, pin definitions, constants
├── adc.c / adc.h             # MQ-3 ADC driver + BAC conversion
├── lcd.c / lcd.h             # 1602A OLED serial display driver
├── pwm.c / pwm.h             # Servo motor bit-bang PWM driver
├── breathalyzer_flowchart.png
├── Demo presentation.pptx
└── ECE 47100 Final Report.docx
```

---

## Course Context

Developed as a final project for **ECE 47100** (Microprocessor System Design) at Purdue University. The project demonstrates interrupt-driven embedded design, hardware peripheral integration (ADC, serial display, servo), and resource-constrained firmware development on an 8-bit PIC microcontroller.

---
