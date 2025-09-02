
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
