/**
 * @file encoder_ec11.c
 * @brief Implementation of EC11 rotary encoder driver
 */

#include "encoder_ec11.h"
#include "hardware/irq.h"

// =============================================================================
// Private Variables
// =============================================================================

// Store encoder instances for ISR access (max 4 encoders)
#define MAX_ENCODERS 4
static encoder_ec11_t *encoder_instances[MAX_ENCODERS] = {NULL};
static uint8_t num_encoders = 0;

// =============================================================================
// Private Functions
// =============================================================================

/**
 * Find encoder instance by pin
 */
static encoder_ec11_t* find_encoder_by_pin(uint gpio) {
    for (int i = 0; i < num_encoders; i++) {
        if (encoder_instances[i]) {
            if (encoder_instances[i]->config.pin_a == gpio ||
                encoder_instances[i]->config.pin_b == gpio ||
                encoder_instances[i]->config.pin_button == gpio) {
                return encoder_instances[i];
            }
        }
    }
    return NULL;
}

/**
 * Update encoder position based on state transition
 */
static void update_position(encoder_ec11_t *encoder, encoder_state_t new_state) {
    // State transition table for quadrature decoding
    // Each transition can be CW (+1), CCW (-1), or invalid (0)
    // CW sequence: 00→01→11→10→00 (should increment)
    // CCW sequence: 00→10→11→01→00 (should decrement)
    static const int8_t state_table[4][4] = {
        //         to: 00  01  10  11
        /* from 00 */ { 0, -1, +1,  0},  // 00→01=CCW, 00→10=CW
        /* from 01 */ {+1,  0,  0, -1},  // 01→00=CW, 01→11=CCW
        /* from 10 */ {-1,  0,  0, +1},  // 10→00=CCW, 10→11=CW  
        /* from 11 */ { 0, +1, -1,  0}   // 11→01=CW, 11→10=CCW
    };
    int8_t delta = state_table[encoder->state][new_state];
    
    if (delta != 0) {
        if (encoder->config.invert_direction) {
            delta = -delta;
        }
        
        encoder->position += delta;
        
        // Debug output
        #ifdef ENCODER_DEBUG
        DEBUG_PRINT("Encoder: %d->%d, delta=%d, pos=%ld", 
                    encoder->state, new_state, delta, encoder->position);
        #endif
        
        // Apply limits if set (max_pos of 0 means no limits)
        if (encoder->max_pos > encoder->min_pos) {
            if (encoder->position > encoder->max_pos) {
                if (encoder->wrap_around) {
                    encoder->position = encoder->min_pos;
                } else {
                    encoder->position = encoder->max_pos;
                }
            } else if (encoder->position < encoder->min_pos) {
                if (encoder->wrap_around) {
                    encoder->position = encoder->max_pos;
                } else {
                    encoder->position = encoder->min_pos;
                }
            }
        }
        
        // Call callback if set
        if (encoder->event_callback) {
            encoder_event_t event = (delta > 0) ? ENCODER_EVENT_CW : ENCODER_EVENT_CCW;
            encoder->event_callback(event, encoder->position);
        }
    }
    
    encoder->state = new_state;
}

/**
 * GPIO interrupt handler
 */
static void gpio_callback(uint gpio, uint32_t events) {
    encoder_ec11_t *encoder = find_encoder_by_pin(gpio);
    if (!encoder) return;
    
    uint64_t now = hw_time_us();
    
    // Handle encoder rotation
    if (gpio == encoder->config.pin_a || gpio == encoder->config.pin_b) {
        // Skip debounce for encoder rotation - quadrature signals are clean
        // and we need maximum responsiveness for fast rotation
        
        // Read current state
        bool a = gpio_get(encoder->config.pin_a);
        bool b = gpio_get(encoder->config.pin_b);
        encoder_state_t new_state = (a << 1) | b;
        
        // Update position
        update_position(encoder, new_state);
    }
    
    // Handle button press
    if (gpio == encoder->config.pin_button) {
        // Debounce
        if ((now - encoder->button_change_us) < encoder->config.button_debounce_us) {
            return;
        }
        encoder->button_change_us = now;
        
        bool pressed = !gpio_get(encoder->config.pin_button);  // Active low
        
        if (pressed != encoder->button_pressed) {
            encoder->button_pressed = pressed;
            
            if (encoder->event_callback) {
                encoder_event_t event = pressed ? ENCODER_EVENT_BUTTON_PRESS : 
                                                 ENCODER_EVENT_BUTTON_RELEASE;
                encoder->event_callback(event, encoder->position);
            }
        }
    }
}

// =============================================================================
// Public Functions
// =============================================================================

hw_result_t encoder_ec11_init(encoder_ec11_t *encoder, const encoder_config_t *config) {
    if (!encoder || !config) {
        return HW_INVALID_PARAM;
    }
    
    if (num_encoders >= MAX_ENCODERS) {
        return HW_ERROR;
    }
    
    // Copy configuration
    encoder->config = *config;
    
    // Set default debounce times if not specified
    if (encoder->config.debounce_us == 0) {
        encoder->config.debounce_us = ENCODER_DEFAULT_DEBOUNCE_US;
    }
    if (encoder->config.button_debounce_us == 0) {
        encoder->config.button_debounce_us = ENCODER_DEFAULT_BUTTON_DEBOUNCE_US;
    }
    
    // Initialize state
    encoder->position = 0;
    encoder->min_pos = 0;
    encoder->max_pos = 0;
    encoder->wrap_around = false;
    encoder->last_change_us = 0;
    encoder->button_pressed = false;
    encoder->button_change_us = 0;
    encoder->event_callback = NULL;
    
    // Initialize GPIO pins for encoder
    if (config->pull_up) {
        hw_gpio_init_input_pullup(config->pin_a);
        hw_gpio_init_input_pullup(config->pin_b);
    } else {
        hw_gpio_init_input_pulldown(config->pin_a);
        hw_gpio_init_input_pulldown(config->pin_b);
    }
    
    // Initialize button pin if used
    if (config->pin_button != (uint)-1) {
        if (config->pull_up) {
            hw_gpio_init_input_pullup(config->pin_button);
        } else {
            hw_gpio_init_input_pulldown(config->pin_button);
        }
    }
    
    // Read initial state
    bool a = gpio_get(config->pin_a);
    bool b = gpio_get(config->pin_b);
    encoder->state = (a << 1) | b;
    
    // Store instance for ISR access
    encoder_instances[num_encoders++] = encoder;
    
    return HW_OK;
}

void encoder_ec11_deinit(encoder_ec11_t *encoder) {
    if (!encoder) return;
    
    // Disable interrupts
    encoder_ec11_disable_interrupts(encoder);
    
    // Remove from instances
    for (int i = 0; i < num_encoders; i++) {
        if (encoder_instances[i] == encoder) {
            // Shift remaining instances
            for (int j = i; j < num_encoders - 1; j++) {
                encoder_instances[j] = encoder_instances[j + 1];
            }
            encoder_instances[--num_encoders] = NULL;
            break;
        }
    }
}

int32_t encoder_ec11_get_position(encoder_ec11_t *encoder) {
    return encoder ? encoder->position : 0;
}

void encoder_ec11_set_position(encoder_ec11_t *encoder, int32_t position) {
    if (encoder) {
        encoder->position = position;
    }
}

void encoder_ec11_reset(encoder_ec11_t *encoder) {
    encoder_ec11_set_position(encoder, 0);
}

void encoder_ec11_set_limits(encoder_ec11_t *encoder, int32_t min, int32_t max, bool wrap) {
    if (encoder) {
        encoder->min_pos = min;
        encoder->max_pos = max;
        encoder->wrap_around = wrap;
    }
}

bool encoder_ec11_button_pressed(encoder_ec11_t *encoder) {
    return encoder ? encoder->button_pressed : false;
}

void encoder_ec11_set_callback(encoder_ec11_t *encoder, 
                               void (*callback)(encoder_event_t, int32_t)) {
    if (encoder) {
        encoder->event_callback = callback;
    }
}

encoder_event_t encoder_ec11_poll(encoder_ec11_t *encoder) {
    if (!encoder) return ENCODER_EVENT_NONE;
    
    static int32_t last_position = 0;
    static bool last_button = false;
    
    // Check for rotation
    int32_t current_pos = encoder->position;
    if (current_pos != last_position) {
        last_position = current_pos;
        return (current_pos > last_position) ? ENCODER_EVENT_CW : ENCODER_EVENT_CCW;
    }
    
    // Check for button
    bool current_button = encoder->button_pressed;
    if (current_button != last_button) {
        last_button = current_button;
        return current_button ? ENCODER_EVENT_BUTTON_PRESS : ENCODER_EVENT_BUTTON_RELEASE;
    }
    
    return ENCODER_EVENT_NONE;
}

int32_t encoder_ec11_get_delta(encoder_ec11_t *encoder) {
    if (!encoder) return 0;
    
    static int32_t last_position = 0;
    int32_t current_pos = encoder->position;
    int32_t delta = current_pos - last_position;
    last_position = current_pos;
    
    return delta;
}

hw_result_t encoder_ec11_enable_interrupts(encoder_ec11_t *encoder) {
    if (!encoder) return HW_INVALID_PARAM;
    
    // Enable interrupts on both edges for encoder pins
    // Both pins need to trigger the callback to catch all state transitions
    gpio_set_irq_enabled_with_callback(encoder->config.pin_a, 
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                                       true, gpio_callback);
    gpio_set_irq_enabled_with_callback(encoder->config.pin_b, 
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                                       true, gpio_callback);
    
    // Enable button interrupt if used
    if (encoder->config.pin_button != (uint)-1) {
        gpio_set_irq_enabled(encoder->config.pin_button, 
                             GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                             true);
    }
    
    return HW_OK;
}

void encoder_ec11_disable_interrupts(encoder_ec11_t *encoder) {
    if (!encoder) return;
    
    // Disable all interrupts
    gpio_set_irq_enabled(encoder->config.pin_a, 
                         GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                         false);
    gpio_set_irq_enabled(encoder->config.pin_b, 
                         GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                         false);
    
    if (encoder->config.pin_button != (uint)-1) {
        gpio_set_irq_enabled(encoder->config.pin_button, 
                             GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                             false);
    }
}