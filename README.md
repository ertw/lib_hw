
# Pico Hardware Library

A collection of hardware peripheral drivers for the Raspberry Pi Pico/Pico 2 W, providing simple interfaces for common sensors and actuators.

## Prerequisites

- **Raspberry Pi Pico SDK** - v2.0+ required for Pico 2 W support
  ```bash
  git clone https://github.com/raspberrypi/pico-sdk.git
  cd pico-sdk && git submodule update --init
  
  # Add to your shell profile (~/.zshrc or ~/.bashrc):
  export PICO_SDK_PATH=/path/to/pico-sdk
  ```
- **ARM GCC Toolchain** - `arm-none-eabi-gcc` (v13.2 or later recommended)
- **CMake** - v3.13 or later
- **Ninja** - Build system

### macOS Setup (Homebrew)

This project has been tested on macOS with tools installed via Homebrew:
```bash
brew install cmake ninja gcc-arm-embedded
```

## Peripherals Supported

- **Button** - Debounced input with press/release detection
- **Rotary Encoder** - EC11 encoder with direction and button support
- **OLED Display** - SH1106 128x64 I2C display driver
- **Stepper Motor** - 28BYJ-48 motor control via ULN2003 driver

## Build

```bash
make        # builds everything (same as 'make build')
```

The target board (pico2_w) is configured in `CMakeLists.txt` line 3.

## Build Commands

```bash
make build       # configure + build (Release)
make debug       # Debug build
make flash       # copy UF2 to BOOTSEL volume (auto-detects RPI-RP2350/RPI-RP2)
make size        # print ELF size
make clean       # cmake clean
make distclean   # delete build dir
make reconfigure # reconfigure cmake (required after CMakeLists.txt changes)
```

## Demos

Example programs are provided in the `demos/` directory for each peripheral.
