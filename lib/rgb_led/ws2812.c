#include "../lib.h"
#include "ws2812.pio.h"
#include <stdlib.h>
#include <string.h>

// WS2812 LED protocol timing constants (in microseconds)
#define WS2812_RESET_DELAY_US 300

// Internal helper functions
static inline uint32_t grb_color(uint8_t r, uint8_t g, uint8_t b) {
    // WS2812 expects data in GRB format
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

static inline void put_pixel(PIO pio, uint sm, uint32_t color) {
    // Left shift by 8 to align with PIO's 32-bit output
    pio_sm_put_blocking(pio, sm, color << 8);
}

hw_result_t hw_ws2812_init(hw_ws2812_t *ws, const hw_ws2812_config_t *config) {
    if (!ws || !config) {
        return HW_INVALID_PARAM;
    }

    // Allocate pixel buffer
    ws->pixels = (uint32_t *)malloc(config->num_pixels * sizeof(uint32_t));
    if (!ws->pixels) {
        return HW_ERROR;
    }

    // Clear pixel buffer
    memset(ws->pixels, 0, config->num_pixels * sizeof(uint32_t));

    // Initialize PIO program
    uint offset = pio_add_program(config->pio, &ws2812_program);
    ws2812_program_init(config->pio, config->sm, offset, config->data_pin, 800000, false);

    // Store config and mark as initialized
    ws->config = config;
    ws->initialized = true;
    
    // Send initial reset signal to ensure LEDs are ready
    hw_sleep_us(WS2812_RESET_DELAY_US);

    return HW_OK;
}

hw_result_t hw_ws2812_set_pixel(hw_ws2812_t *ws, uint index, uint8_t r, uint8_t g, uint8_t b) {
    if (!ws || !ws->initialized || index >= ws->config->num_pixels) {
        return HW_INVALID_PARAM;
    }

    ws->pixels[index] = grb_color(r, g, b);
    return HW_OK;
}

hw_result_t hw_ws2812_set_all(hw_ws2812_t *ws, uint8_t r, uint8_t g, uint8_t b) {
    if (!ws || !ws->initialized) {
        return HW_INVALID_PARAM;
    }

    uint32_t color = grb_color(r, g, b);
    for (uint i = 0; i < ws->config->num_pixels; i++) {
        ws->pixels[i] = color;
    }
    return HW_OK;
}

hw_result_t hw_ws2812_show(hw_ws2812_t *ws) {
    if (!ws || !ws->initialized) {
        return HW_INVALID_PARAM;
    }

    // Send each pixel's color to the LED strip
    for (uint i = 0; i < ws->config->num_pixels; i++) {
        put_pixel(ws->config->pio, ws->config->sm, ws->pixels[i]);
    }

    // Latch colors with reset delay
    hw_sleep_us(WS2812_RESET_DELAY_US);
    return HW_OK;
}

hw_result_t hw_ws2812_clear(hw_ws2812_t *ws) {
    return hw_ws2812_set_all(ws, 0, 0, 0);
}