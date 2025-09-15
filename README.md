
# Pico Hardware Library

A collection of hardware peripheral drivers for the Raspberry Pi Pico/Pico 2 W, providing simple interfaces for common sensors and actuators.

## Peripherals Supported

- **Button** - Debounced input with press/release detection
- **Rotary Encoder** - EC11 encoder with direction and button support
- **OLED Display** - SH1106 128x64 I2C display driver
- **Stepper Motor** - 28BYJ-48 motor control via ULN2003 driver

## Build

```bash
mkdir build && cd build
cmake -DPICO_BOARD=pico2_w ..
make -j
```

## Quick Build Commands

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
