
# Pico 2 W + 28BYJ-48 Stepper Demo (ULN2003)
Spin 3s CW, 3s CCW, then de-energize coils. Default pins: IN1..IN4 = GP2..GP5.

## Build (Pico SDK v2+)
mkdir build && cd build
cmake -DPICO_BOARD=pico2_w ..
make -j

## Makefile workflow
make build       # configure + build (Release)
make debug       # Debug build
make flash       # copy UF2 to BOOTSEL volume (auto-detects RPI-RP2350/RPI-RP2)
make size        # print ELF size
make clean       # cmake clean
make distclean   # delete build dir
