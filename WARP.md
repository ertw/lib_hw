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

## Hardware Components Interfaced

### 28BYJ-48 Stepper Motor
- 5V geared stepper motor with 2048 steps per revolution
- Controlled via ULN2003 driver board (Darlington transistor array)
- Implemented using half-stepping sequence (8 steps) for smooth rotation
- Features automatic coil de-energization for power saving

### Push Buttons
- Tactile switches for user input
- Configured with internal pull-up resistors (active-low logic)
- Used for directional control and other inputs

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
