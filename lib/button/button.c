/**
 * @file button.c
 * @brief Implementation of button driver with debouncing
 */

#include "../lib.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"

// =============================================================================
// Private Variables
// =============================================================================

// Store button instances for ISR access (max 8 buttons)
#define MAX_BUTTONS 8
static button_t *button_instances[MAX_BUTTONS] = {NULL};
static uint8_t num_buttons = 0;

// =============================================================================
// Private Functions
// =============================================================================

/**
 * Find button instance by pin
 */
static button_t* find_button_by_pin(uint gpio) {
    for (int i = 0; i < num_buttons; i++) {
        if (button_instances[i] && button_instances[i]->config.pin == gpio) {
            return button_instances[i];
        }
    }
    return NULL;
}

/**
 * Read button pin state accounting for active_low
 */
static bool read_button_state(button_t *button) {
    bool pin_state = gpio_get(button->config.pin);
    return button->config.active_low ? !pin_state : pin_state;
}

/**
 * Process button state change
 * Note: This is called from button_poll() in normal context, NOT from interrupt!
 * Callbacks fired here are safe to do complex operations.
 */
static void process_state_change(button_t *button, bool new_state, uint64_t now) {
    button->debounced_state = new_state;
    
    if (new_state) {  // Button pressed
        button->state = BUTTON_STATE_PRESSED;
        button->press_start_time = now;
        button->long_press_fired = false;
        
        if (button->event_callback) {
            button->event_callback(BUTTON_EVENT_PRESS, button->click_count);
        }
    } else {  // Button released
        button->state = BUTTON_STATE_RELEASED;
        
        // Check if this was a normal click (not long press)
        uint64_t press_duration = now - button->press_start_time;
        // Only check long press timing if long press is enabled or if we need to distinguish for multi-click
        bool is_short_press = true;
        if (button->config.enable_long_press || button->config.enable_multi_click) {
            is_short_press = press_duration < MS_TO_US(button->config.long_press_ms);
        }
        
        if (is_short_press && !button->long_press_fired) {
            // If multi-click is disabled, emit CLICK immediately
            if (!button->config.enable_multi_click) {
                button->pending_event = BUTTON_EVENT_CLICK;
                button->pending_clicks = 1;
                button->click_count = 0;
            } else {
                // Increment click count and wait for timeout to decide single/double/triple
                button->click_count++;
                button->last_click_time = now;
            }
        }
        
        if (button->event_callback) {
            button->event_callback(BUTTON_EVENT_RELEASE, button->click_count);
        }
    }
}

/**
 * Check for long press
 */
static void check_long_press(button_t *button, uint64_t now) {
    if (button->state == BUTTON_STATE_PRESSED && !button->long_press_fired && button->config.enable_long_press) {
        uint64_t press_duration = now - button->press_start_time;
        if (press_duration >= MS_TO_US(button->config.long_press_ms)) {
            button->long_press_fired = true;
            button->state = BUTTON_STATE_LONG_PRESSED;
            button->click_count = 0;  // Reset click count on long press
            
            if (button->event_callback) {
                button->event_callback(BUTTON_EVENT_LONG_PRESS, 0);
            }
        }
    }
}

/**
 * Check for completed click sequence
 */
static button_event_t check_click_sequence(button_t *button, uint64_t now) {
    if (!button->config.enable_multi_click) {
        return BUTTON_EVENT_NONE;
    }
    if (button->click_count > 0 && button->state == BUTTON_STATE_IDLE) {
        uint64_t time_since_click = now - button->last_click_time;
        if (time_since_click >= MS_TO_US(button->config.multi_click_ms)) {
            // Multi-click timeout reached, fire appropriate event
            button_event_t event = BUTTON_EVENT_NONE;
            uint8_t clicks = button->click_count;
            
            switch (clicks) {
                case 1:
                    event = BUTTON_EVENT_CLICK;
                    break;
                case 2:
                    event = BUTTON_EVENT_DOUBLE_CLICK;
                    break;
                case 3:
                    event = BUTTON_EVENT_TRIPLE_CLICK;
                    break;
                default:
                    // More than 3 clicks, report as triple click
                    event = BUTTON_EVENT_TRIPLE_CLICK;
                    clicks = 3;
                    break;
            }
            
            button->click_count = 0;  // Reset for next sequence
            
            if (button->event_callback && event != BUTTON_EVENT_NONE) {
                button->event_callback(event, clicks);
            }
            
            return event;
        }
    }
    return BUTTON_EVENT_NONE;
}

/**
 * GPIO interrupt handler
 * Note: This runs in interrupt context - keep it minimal!
 * Only updates raw state; actual processing happens in button_poll()
 */
static void gpio_callback(uint gpio, uint32_t events) {
    button_t *button = find_button_by_pin(gpio);
    if (!button) return;
    
    // Update raw state and timestamp atomically
    uint32_t save = save_and_disable_interrupts();
    button->raw_state = read_button_state(button);
    button->state_change_time = hw_time_us();
    restore_interrupts(save);
}

// =============================================================================
// Public Functions
// =============================================================================

hw_result_t button_init(button_t *button, const button_config_t *config) {
    if (!button || !config) {
        return HW_INVALID_PARAM;
    }
    
    if (num_buttons >= MAX_BUTTONS) {
        DEBUG_PRINT("Button init failed: max buttons (%d) reached", MAX_BUTTONS);
        return HW_ERROR;
    }
    
    // Validate GPIO pin number (Pico has 30 GPIOs: 0-29)
    if (config->pin >= 30) {
        DEBUG_PRINT("Button init failed: invalid pin %u", config->pin);
        return HW_INVALID_PARAM;
    }
    
    // Copy configuration
    button->config = *config;
    
    // Set default timing values if not specified
    if (button->config.debounce_ms == 0) {
        button->config.debounce_ms = BUTTON_DEFAULT_DEBOUNCE_MS;
    }
    if (button->config.long_press_ms == 0) {
        button->config.long_press_ms = BUTTON_DEFAULT_LONG_PRESS_MS;
    }
    if (button->config.multi_click_ms == 0) {
        button->config.multi_click_ms = BUTTON_DEFAULT_MULTI_CLICK_MS;
    }

    // Initialize GPIO
    if (config->pull_up) {
        hw_gpio_init_input_pullup(config->pin);
    } else {
        hw_gpio_init_input_pulldown(config->pin);
    }
    
    // Initialize state
    button->raw_state = read_button_state(button);
    button->debounced_state = button->raw_state;
    button->state = button->debounced_state ? BUTTON_STATE_PRESSED : BUTTON_STATE_IDLE;
    button->state_change_time = hw_time_us();
    button->click_count = 0;
    button->last_click_time = 0;
    button->press_start_time = 0;
    button->long_press_fired = false;
    button->event_callback = NULL;
    button->pending_event = BUTTON_EVENT_NONE;
    button->pending_clicks = 0;
    
    // Store instance for ISR access (with critical section for thread safety)
    uint32_t save = save_and_disable_interrupts();
    button_instances[num_buttons++] = button;
    restore_interrupts(save);
    
    return HW_OK;
}

void button_deinit(button_t *button) {
    if (!button) return;
    
    // Disable interrupts
    button_disable_interrupts(button);
    
    // Remove from instances (with critical section for thread safety)
    uint32_t save = save_and_disable_interrupts();
    for (int i = 0; i < num_buttons; i++) {
        if (button_instances[i] == button) {
            // Shift remaining instances
            for (int j = i; j < num_buttons - 1; j++) {
                button_instances[j] = button_instances[j + 1];
            }
            button_instances[--num_buttons] = NULL;
            break;
        }
    }
    restore_interrupts(save);
}

button_event_t button_poll(button_t *button) {
    if (!button) return BUTTON_EVENT_NONE;
    
    uint64_t now = hw_time_us();
    
    // Read raw state with interrupts disabled to ensure consistency
    uint32_t save = save_and_disable_interrupts();
    bool current_raw = button->raw_state;
    uint64_t state_change_time = button->state_change_time;
    restore_interrupts(save);
    
    button_event_t event = BUTTON_EVENT_NONE;
    
    // If we have a pending event (e.g., immediate CLICK), return it once
    if (button->pending_event != BUTTON_EVENT_NONE) {
        event = button->pending_event;
        if (button->event_callback) {
            button->event_callback(event, button->pending_clicks);
        }
        button->pending_event = BUTTON_EVENT_NONE;
        button->pending_clicks = 0;
        return event;
    }

    // Check if debounce period has elapsed
    uint64_t time_since_change = now - state_change_time;
    if (time_since_change >= MS_TO_US(button->config.debounce_ms)) {
        // Check if state has changed
        if (current_raw != button->debounced_state) {
            process_state_change(button, current_raw, now);
        }
    }
    
    // Update state to idle if released
    if (button->state == BUTTON_STATE_RELEASED && !button->debounced_state) {
        button->state = BUTTON_STATE_IDLE;
    }
    
    // Check for long press
    check_long_press(button, now);
    
    // Check for completed click sequence
    event = check_click_sequence(button, now);
    
    return event;
}

bool button_is_pressed(button_t *button) {
    return button ? button->debounced_state : false;
}

bool button_get_raw_state(button_t *button) {
    if (!button) return false;
    
    // Update raw state from pin
    button->raw_state = read_button_state(button);
    return button->raw_state;
}

uint8_t button_get_click_count(button_t *button) {
    return button ? button->click_count : 0;
}

void button_reset(button_t *button) {
    if (!button) return;
    
    button->click_count = 0;
    button->last_click_time = 0;
    button->long_press_fired = false;
    button->pending_event = BUTTON_EVENT_NONE;
    button->pending_clicks = 0;
    button->state = button->debounced_state ? BUTTON_STATE_PRESSED : BUTTON_STATE_IDLE;
}

void button_set_callback(button_t *button, 
                        void (*callback)(button_event_t, uint8_t)) {
    if (button) {
        button->event_callback = callback;
    }
}

hw_result_t button_enable_interrupts(button_t *button) {
    if (!button) return HW_INVALID_PARAM;
    
    // Ensure initial state is synchronized before enabling interrupts
    button->raw_state = read_button_state(button);
    button->state_change_time = hw_time_us();
    
    // Enable interrupts on both edges
    gpio_set_irq_enabled_with_callback(button->config.pin, 
                                      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                                      true, gpio_callback);
    
    return HW_OK;
}

void button_disable_interrupts(button_t *button) {
    if (!button) return;
    
    gpio_set_irq_enabled(button->config.pin, 
                        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                        false);
}

void button_set_timing(button_t *button, uint32_t debounce_ms, 
                      uint32_t long_press_ms, uint32_t multi_click_ms) {
    if (!button) return;
    
    if (debounce_ms > 0) {
        button->config.debounce_ms = debounce_ms;
    }
    if (long_press_ms > 0) {
        button->config.long_press_ms = long_press_ms;
    }
    if (multi_click_ms > 0) {
        button->config.multi_click_ms = multi_click_ms;
    }
}
