# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

This is a Raspberry Pi Pico 2 W experimentation and learning platform. The codebase demonstrates various hardware interfacing techniques and embedded programming concepts.

## Prerequisites

- **PICO_SDK_PATH** environment variable must be set to your Pico SDK location
  - Current: `/Users/erikwilliamson/pico/pico-sdk`
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
