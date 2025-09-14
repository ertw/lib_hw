/**
 * @file lib.h
 * @brief Main library header for Pico hardware peripheral drivers
 * 
 * This header provides common definitions, macros, and type definitions
 * shared across all peripheral drivers for the Raspberry Pi Pico.
 */

#ifndef PICO_HW_LIB_H
#define PICO_HW_LIB_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/time.h"

// =============================================================================
// Common Macros
// =============================================================================

/** Helper macro to get array size */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/** Min/Max macros */
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

/** Constrain value between min and max */
#define CONSTRAIN(val, min, max) (MIN(MAX(val, min), max))

/** Bit manipulation macros */
#define BIT(n) (1UL << (n))
#define SET_BIT(reg, bit) ((reg) |= BIT(bit))
#define CLEAR_BIT(reg, bit) ((reg) &= ~BIT(bit))
#define TOGGLE_BIT(reg, bit) ((reg) ^= BIT(bit))
#define CHECK_BIT(reg, bit) ((reg) & BIT(bit))

/** Debug print macro - can be disabled by defining NO_DEBUG */
#ifdef NO_DEBUG
#define DEBUG_PRINT(fmt, ...) ((void)0)
#else
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#endif

// =============================================================================
// Common Type Definitions
// =============================================================================

/** Generic result type for library functions */
typedef enum {
    HW_OK = 0,           ///< Operation successful
    HW_ERROR = -1,       ///< General error
    HW_TIMEOUT = -2,     ///< Operation timed out
    HW_BUSY = -3,        ///< Device busy
    HW_INVALID_PARAM = -4, ///< Invalid parameter
    HW_NOT_FOUND = -5,   ///< Device not found
} hw_result_t;

/** Common direction enumeration */
typedef enum {
    DIR_FORWARD = 0,
    DIR_BACKWARD = 1,
    DIR_CW = 0,          // Clockwise (alias for forward)
    DIR_CCW = 1,         // Counter-clockwise (alias for backward)
} hw_direction_t;

/** Common state enumeration */
typedef enum {
    STATE_OFF = 0,
    STATE_ON = 1,
    STATE_IDLE = 2,
    STATE_ACTIVE = 3,
    STATE_ERROR = 4,
} hw_state_t;

// =============================================================================
// Timing Utilities
// =============================================================================

/** Convert milliseconds to microseconds */
#define MS_TO_US(ms) ((ms) * 1000ULL)

/** Sleep for specified milliseconds */
static inline void hw_sleep_ms(uint32_t ms) {
    sleep_ms(ms);
}

/** Sleep for specified microseconds */
static inline void hw_sleep_us(uint64_t us) {
    sleep_us(us);
}

/** Get current time in microseconds */
static inline uint64_t hw_time_us(void) {
    return to_us_since_boot(get_absolute_time());
}

/** Check if timeout has elapsed */
static inline bool hw_timeout_elapsed(uint64_t start_us, uint64_t timeout_us) {
    return (hw_time_us() - start_us) >= timeout_us;
}

// =============================================================================
// GPIO Utilities  
// =============================================================================

/** Initialize GPIO as input with pull-up */
static inline void hw_gpio_init_input_pullup(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

/** Initialize GPIO as input with pull-down */
static inline void hw_gpio_init_input_pulldown(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_down(gpio);
}

/** Initialize GPIO as output */
static inline void hw_gpio_init_output(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
}

/** Initialize GPIO as output with initial value */
static inline void hw_gpio_init_output_val(uint gpio, bool value) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, value);
}

// =============================================================================
// I2C Utilities
// =============================================================================

/** I2C configuration structure */
typedef struct {
    i2c_inst_t *instance;  ///< I2C instance (i2c0 or i2c1)
    uint sda_pin;          ///< SDA pin number
    uint scl_pin;          ///< SCL pin number
    uint baudrate;         ///< I2C baudrate in Hz
} hw_i2c_config_t;

/** Initialize I2C with configuration */
static inline hw_result_t hw_i2c_init(const hw_i2c_config_t *config) {
    if (!config) return HW_INVALID_PARAM;
    
    i2c_init(config->instance, config->baudrate);
    gpio_set_function(config->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(config->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(config->sda_pin);
    gpio_pull_up(config->scl_pin);
    
    return HW_OK;
}

/** Scan I2C bus for devices */
static inline hw_result_t hw_i2c_scan(i2c_inst_t *i2c, uint8_t *found_addrs, uint8_t *count, uint8_t max_count) {
    if (!i2c || !found_addrs || !count) return HW_INVALID_PARAM;
    
    *count = 0;
    uint8_t rxdata;
    
    for (uint8_t addr = 0x08; addr < 0x78 && *count < max_count; addr++) {
        if (i2c_read_blocking(i2c, addr, &rxdata, 1, false) >= 0) {
            found_addrs[(*count)++] = addr;
        }
    }
    
    return (*count > 0) ? HW_OK : HW_NOT_FOUND;
}

// =============================================================================
// Module Includes
// =============================================================================

// Include individual peripheral driver headers
#include "oled/sh1106.h"
#include "stepper/stepper_28byj48.h"
#include "encoder/encoder_ec11.h"

#endif // PICO_HW_LIB_H
