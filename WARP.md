# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

This is a Raspberry Pi Pico 2 W experimentation and learning platform. The codebase demonstrates various hardware interfacing techniques and embedded programming concepts.

## Hardware Platform (Wukong2040)

This project uses the Elecfreaks Wukong2040 breakout board with a Raspberry Pi Pico 2 W.

### On-board peripherals (from pinout image)
- Rainbow LED (WS2812 data): GP22
- Button A: GP18
- Button B: GP19
- Buzzer: GP9
- DC Motor ports (white JST headers at bottom):
  - M1: M1+ → GP21, M1− → GP20
  - M2: M2+ → GP11, M2− → GP10
- RGB headers (top-right, labeled M3/M4):
  - M3 → GP12/GP13
  - M4 → GP14/GP15

### Accessible GPIO headers
The Wukong2040 breaks out most GPIOs on three vertical header columns with silkscreened alternate functions. Convenient groups used by this repo:

- Top-right group:
  - GP28, GP27, GP26 (ADC2/ADC1/ADC0)
  - GP8, GP7, GP6 (GPIO — GP7/GP6 are I2C1 SCL/SDA used by OLED)
- Mid-right group:
  - GP16, GP17 (also labeled SPI0 RX / SPI0 CSn)
- Bottom-right group:
  - GP5, GP4, GP3, GP2, GP1, GP0 (with SPI0/I2C0/UART alt functions on silkscreen)

### I2C/SPI/UART quick reference from board labels
- I2C1: GP7 (SCL), GP6 (SDA) — used by OLED
- I2C0: GP21 (SCL), GP20 (SDA) present on headers (also used by M1, so avoid when motors connected)
- ADC: GP26=ADC0, GP27=ADC1, GP28=ADC2
- SPI0 signals appear on multiple header pads (SCK, TX, RX, CSn) per silkscreen
- UART0/1 TX/RX are available on several header pads per silkscreen

### Recommended wiring used in this repo
- OLED (SH1106): I2C1 on GP6 (SDA) / GP7 (SCL), VCC=5V (VBUS), GND
- EC11 encoder: A→GP26, B→GP27, Push→GP28 (3V3 and GND as appropriate)

Note: Several pins are shared with on-board peripherals (e.g., motors, buzzer). Avoid reusing those when the peripherals are in use. Always refer to the pinout image for exact alternate-function locations.

## Prerequisites
- Pico SDK v2.0 or higher required
- Ninja build system
- ARM GCC toolchain for RP2350

## Common Development Commands

### Building
```bash
make build              # Release build (default)
make debug              # Debug build
make reconfigure        # Force CMake reconfiguration
```

**IMPORTANT**: After modifying CMakeLists.txt, you MUST run `make reconfigure` before `make build`. The build system does not automatically detect CMake changes, so running only `make build` will use the old configuration.

### Flashing to Device
```bash
make flash              # Auto-detect and copy to BOOTSEL volume (RPI-RP2350/RPI-RP2)
```

### Cleaning
```bash
make clean              # Clean build artifacts (ninja clean)
make distclean          # Remove entire build directory
```

### Debugging & Analysis
```bash
make size               # Display ELF binary size information
make serial             # Connect to Pico serial port with minicom
```

## Architecture & Code Structure

### Build System
- **Makefile**: Wrapper around CMake/Ninja for convenience commands, handles build type configuration
- **CMakeLists.txt**: Configures the Pico SDK build, targets `pico2_w` board
- **pico_sdk_import.cmake**: Imports Pico SDK from environment variable

### Main Application
The `src/main.c` file contains the primary application logic. Pin assignments and configuration constants are defined at the top of the source file for easy modification during experimentation.

## Available Demos

### 1. Stepper Motor Demo (`steppydemo.uf2`)
Controls a 28BYJ-48 stepper motor with push buttons.
- **Motor**: 28BYJ-48 5V stepper with ULN2003 driver
- **Control**: Push buttons on GP18 (CW) and GP19 (CCW)
- **Features**: Half-stepping, automatic coil de-energization

### 2. OLED Display Demo (`oled_demo.uf2`)
Demonstrates SH1106 1.3" OLED display capabilities.
- **Display**: 128x64 pixels, I2C interface at address 0x3C
- **Wiring**: SDA→GP6, SCL→GP7, VCC→5V (VBUS), GND→GND
- **Features**: Text rendering, graphics primitives, animations
- **Note**: Must use i2c1 (not i2c0) for GP6/GP7 pins

See `OLED_README.md` for detailed OLED setup instructions.

### 3. Rotary Encoder Demo (`encoder_demo.uf2`)
Control a rotating square on the OLED with an EC11 encoder.
- **Encoder**: EC11, 20 detents/rev
- **Wiring**: A→GP26, B→GP27, Push→GP28 (Wukong2040 accessible pins)
- **Display**: Uses same OLED wiring as above (GP6/GP7 I2C1)
- **Features**: Interrupt-driven quadrature decoding, smooth rotation

### Control Techniques Demonstrated
- **Half-stepping**: 8-step sequence for improved smoothness and torque
- **Time-based control**: Non-blocking delays using Pico SDK absolute time functions
- **Power management**: Automatic motor coil shutdown when idle
- **Button debouncing**: Implicit handling through timing constraints

## Build Output

The build system generates multiple output formats:
- **steppydemo.uf2**: Primary flashing format for Pico
- **steppydemo.elf**: Debug symbols and full binary
- **steppydemo.bin**: Raw binary output
- **steppydemo.hex**: Intel HEX format

All outputs are located in the `build/` directory.

## Development Notes

This repository serves as an experimentation platform. Hardware configurations and pin assignments may change frequently. Always refer to the source code for current pin mappings and configuration values. Key parameters are typically defined as macros at the beginning of source files for easy modification.
