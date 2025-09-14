
cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico2_w)  # <-- user requested

include(pico_sdk_import.cmake)

project(pico_28byj_demo C CXX ASM)
pico_sdk_init()

# =============================================================================
# Hardware Library
# =============================================================================

# Create a static library with all the hardware drivers
add_library(pico_hw_lib STATIC
    lib/oled/sh1106.c
    lib/stepper/stepper_28byj48.c
    lib/encoder/encoder_ec11.c
)

# Include directories for the library
target_include_directories(pico_hw_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Link the library with required Pico SDK libraries
target_link_libraries(pico_hw_lib PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_irq
)

# =============================================================================
# Demo Executables
# =============================================================================

# Stepper Motor Demo
add_executable(steppydemo src/main.c)
pico_enable_stdio_usb(steppydemo 1)
pico_enable_stdio_uart(steppydemo 0)
target_link_libraries(steppydemo 
    pico_hw_lib
)
pico_add_extra_outputs(steppydemo)

# OLED Demo
add_executable(oled_demo src/oled_demo.c)
pico_enable_stdio_usb(oled_demo 1)
pico_enable_stdio_uart(oled_demo 0)
target_link_libraries(oled_demo 
    pico_hw_lib
)
pico_add_extra_outputs(oled_demo)

# Rotary Encoder Demo with OLED display
add_executable(encoder_demo src/encoder_demo.c)
pico_enable_stdio_usb(encoder_demo 1)
pico_enable_stdio_uart(encoder_demo 0)
target_link_libraries(encoder_demo 
    pico_hw_lib
)
pico_add_extra_outputs(encoder_demo)
