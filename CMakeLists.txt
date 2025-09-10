
cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico2_w)  # <-- user requested

include(pico_sdk_import.cmake)

project(pico_28byj_demo C CXX ASM)
pico_sdk_init()

add_executable(steppydemo src/main.c)
pico_enable_stdio_usb(steppydemo 1)
pico_enable_stdio_uart(steppydemo 0)
target_link_libraries(steppydemo pico_stdlib)
pico_add_extra_outputs(steppydemo)

# OLED Demo executable with SH1106 driver
add_executable(oled_demo src/oled_demo.c src/sh1106.c)
pico_enable_stdio_usb(oled_demo 1)
pico_enable_stdio_uart(oled_demo 0)
target_link_libraries(oled_demo 
    pico_stdlib 
    hardware_i2c
)
pico_add_extra_outputs(oled_demo)

# Rotary Encoder Demo with OLED display
add_executable(encoder_demo src/encoder_demo.c src/sh1106.c)
pico_enable_stdio_usb(encoder_demo 1)
pico_enable_stdio_uart(encoder_demo 0)
target_link_libraries(encoder_demo 
    pico_stdlib 
    hardware_i2c
    hardware_gpio
)
pico_add_extra_outputs(encoder_demo)
