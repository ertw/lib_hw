/**
 * @file button.h
 * @brief Driver for simple push buttons with debouncing
 * 
 * This driver provides interrupt-driven and polled button reading with
 * software debouncing, multi-click detection, and long-press support.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "../lib.h"

// =============================================================================
// Configuration
// =============================================================================

/** Default debounce time in milliseconds */
#define BUTTON_DEFAULT_DEBOUNCE_MS 50

/** Default long press time in milliseconds */
#define BUTTON_DEFAULT_LONG_PRESS_MS 1000

/** Default multi-click timeout in milliseconds */
#define BUTTON_DEFAULT_MULTI_CLICK_MS 400

/** Maximum number of clicks to track for multi-click */
#define BUTTON_MAX_MULTI_CLICKS 3

// =============================================================================
// Type Definitions
// =============================================================================

/** Button event types */
typedef enum {
    BUTTON_EVENT_NONE = 0,      ///< No event
    BUTTON_EVENT_PRESS,         ///< Button pressed down
    BUTTON_EVENT_RELEASE,       ///< Button released
    BUTTON_EVENT_CLICK,         ///< Single click detected
    BUTTON_EVENT_DOUBLE_CLICK,  ///< Double click detected
    BUTTON_EVENT_TRIPLE_CLICK,  ///< Triple click detected
    BUTTON_EVENT_LONG_PRESS,    ///< Long press detected
} button_event_t;

/** Button state */
typedef enum {
    BUTTON_STATE_IDLE = 0,      ///< Button is idle (not pressed)
    BUTTON_STATE_PRESSED,       ///< Button is currently pressed
    BUTTON_STATE_RELEASED,      ///< Button was just released
    BUTTON_STATE_LONG_PRESSED,  ///< Button is in long press
} button_state_t;

/** Button configuration */
typedef struct {
    uint pin;                   ///< GPIO pin number
    bool active_low;            ///< True if button connects to ground when pressed
    bool pull_up;               ///< Use internal pull-up (true) or pull-down (false)
    uint32_t debounce_ms;       ///< Debounce time in milliseconds
    uint32_t long_press_ms;     ///< Long press threshold in milliseconds
    uint32_t multi_click_ms;    ///< Multi-click timeout in milliseconds
} button_config_t;

/** Button instance */
typedef struct {
    button_config_t config;     ///< Button configuration
    
    // State tracking
    button_state_t state;       ///< Current button state
    bool raw_state;             ///< Raw pin state (before debouncing)
    bool debounced_state;       ///< Debounced pin state
    uint64_t state_change_time; ///< Last state change time
    
    // Click detection
    uint8_t click_count;        ///< Number of clicks detected
    uint64_t last_click_time;   ///< Time of last click
    
    // Long press detection
    uint64_t press_start_time;  ///< Time when button was pressed
    bool long_press_fired;      ///< Long press event already fired
    
    // Event callback
    void (*event_callback)(button_event_t event, uint8_t click_count); ///< Optional event callback
} button_t;

// =============================================================================
// Function Prototypes
// =============================================================================

/**
 * Initialize button
 * @param button Pointer to button instance
 * @param config Pointer to configuration
 * @return HW_OK on success, error code otherwise
 */
hw_result_t button_init(button_t *button, const button_config_t *config);

/**
 * Deinitialize button and remove interrupts
 * @param button Pointer to button instance
 */
void button_deinit(button_t *button);

/**
 * Poll button state (for non-interrupt operation)
 * @param button Pointer to button instance
 * @return Current button event if any
 */
button_event_t button_poll(button_t *button);

/**
 * Get current button state
 * @param button Pointer to button instance
 * @return true if button is pressed
 */
bool button_is_pressed(button_t *button);

/**
 * Get raw button state (before debouncing)
 * @param button Pointer to button instance
 * @return true if button pin is active
 */
bool button_get_raw_state(button_t *button);

/**
 * Get click count for multi-click detection
 * @param button Pointer to button instance
 * @return Number of clicks detected
 */
uint8_t button_get_click_count(button_t *button);

/**
 * Reset button state and click count
 * @param button Pointer to button instance
 */
void button_reset(button_t *button);

/**
 * Set event callback function
 * @param button Pointer to button instance
 * @param callback Callback function (NULL to disable)
 */
void button_set_callback(button_t *button, 
                        void (*callback)(button_event_t, uint8_t));

/**
 * Enable interrupt-driven operation
 * @param button Pointer to button instance
 * @return HW_OK on success
 */
hw_result_t button_enable_interrupts(button_t *button);

/**
 * Disable interrupt-driven operation
 * @param button Pointer to button instance
 */
void button_disable_interrupts(button_t *button);

/**
 * Update button timing configuration
 * @param button Pointer to button instance
 * @param debounce_ms New debounce time (0 to keep current)
 * @param long_press_ms New long press time (0 to keep current)
 * @param multi_click_ms New multi-click timeout (0 to keep current)
 */
void button_set_timing(button_t *button, uint32_t debounce_ms, 
                      uint32_t long_press_ms, uint32_t multi_click_ms);

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * Get event name as string
 * @param event Button event
 * @return String representation of event
 */
static inline const char* button_event_to_string(button_event_t event) {
    switch (event) {
        case BUTTON_EVENT_NONE:         return "NONE";
        case BUTTON_EVENT_PRESS:        return "PRESS";
        case BUTTON_EVENT_RELEASE:      return "RELEASE";
        case BUTTON_EVENT_CLICK:        return "CLICK";
        case BUTTON_EVENT_DOUBLE_CLICK: return "DOUBLE_CLICK";
        case BUTTON_EVENT_TRIPLE_CLICK: return "TRIPLE_CLICK";
        case BUTTON_EVENT_LONG_PRESS:   return "LONG_PRESS";
        default:                        return "UNKNOWN";
    }
}

#endif // BUTTON_H
