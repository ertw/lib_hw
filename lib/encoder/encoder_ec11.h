/**
 * @file encoder_ec11.h
 * @brief Driver for EC11 rotary encoder with push button
 * 
 * This driver provides interrupt-driven quadrature decoding for EC11-style
 * rotary encoders. It handles debouncing, direction detection, and optional
 * push button functionality.
 */

#ifndef ENCODER_EC11_H
#define ENCODER_EC11_H

// =============================================================================
// Configuration
// =============================================================================

/** Default debounce time in microseconds */
#define ENCODER_DEFAULT_DEBOUNCE_US 1000

/** Default button debounce time in microseconds */
#define ENCODER_DEFAULT_BUTTON_DEBOUNCE_US 50000

// =============================================================================
// Type Definitions
// =============================================================================

/** Encoder event types */
typedef enum {
    ENCODER_EVENT_NONE = 0,     ///< No event
    ENCODER_EVENT_CW,           ///< Clockwise rotation
    ENCODER_EVENT_CCW,          ///< Counter-clockwise rotation
    ENCODER_EVENT_BUTTON_PRESS, ///< Button pressed
    ENCODER_EVENT_BUTTON_RELEASE,///< Button released
} encoder_event_t;

/** Encoder configuration */
typedef struct {
    uint pin_a;                 ///< Encoder A pin (CLK)
    uint pin_b;                 ///< Encoder B pin (DT)
    uint pin_button;            ///< Push button pin (SW), set to -1 if not used
    bool invert_direction;      ///< Invert rotation direction
    uint32_t debounce_us;       ///< Encoder debounce time in microseconds
    uint32_t button_debounce_us;///< Button debounce time in microseconds
    bool pull_up;               ///< Use internal pull-ups (true) or pull-downs (false)
} encoder_config_t;

/** Encoder state machine states */
typedef enum {
    ENCODER_STATE_00 = 0,
    ENCODER_STATE_01 = 1,
    ENCODER_STATE_10 = 2,
    ENCODER_STATE_11 = 3,
} encoder_state_t;

/** Encoder instance */
typedef struct {
    encoder_config_t config;    ///< Encoder configuration
    
    // Position tracking
    volatile int32_t position;  ///< Current position (increments/decrements)
    volatile int32_t min_pos;   ///< Minimum position limit (0 = no limit)
    volatile int32_t max_pos;   ///< Maximum position limit (0 = no limit)
    bool wrap_around;           ///< Wrap around at limits
    
    // State tracking
    encoder_state_t state;      ///< Current state machine state
    uint64_t last_change_us;    ///< Last state change time
    
    // Button state
    volatile bool button_pressed; ///< Current button state
    uint64_t button_change_us;  ///< Last button change time
    
    // Event callback
    void (*event_callback)(encoder_event_t event, int32_t position); ///< Optional event callback
} encoder_ec11_t;

// =============================================================================
// Function Prototypes
// =============================================================================

/**
 * Initialize encoder
 * @param encoder Pointer to encoder instance
 * @param config Pointer to configuration
 * @return HW_OK on success, error code otherwise
 */
hw_result_t encoder_ec11_init(encoder_ec11_t *encoder, const encoder_config_t *config);

/**
 * Deinitialize encoder and remove interrupts
 * @param encoder Pointer to encoder instance
 */
void encoder_ec11_deinit(encoder_ec11_t *encoder);

/**
 * Get current position
 * @param encoder Pointer to encoder instance
 * @return Current position value
 */
int32_t encoder_ec11_get_position(encoder_ec11_t *encoder);

/**
 * Set current position
 * @param encoder Pointer to encoder instance
 * @param position New position value
 */
void encoder_ec11_set_position(encoder_ec11_t *encoder, int32_t position);

/**
 * Reset position to zero
 * @param encoder Pointer to encoder instance
 */
void encoder_ec11_reset(encoder_ec11_t *encoder);

/**
 * Set position limits
 * @param encoder Pointer to encoder instance
 * @param min Minimum position (0 = no limit)
 * @param max Maximum position (0 = no limit)
 * @param wrap Whether to wrap around at limits
 */
void encoder_ec11_set_limits(encoder_ec11_t *encoder, int32_t min, int32_t max, bool wrap);

/**
 * Get button state
 * @param encoder Pointer to encoder instance
 * @return true if button is pressed
 */
bool encoder_ec11_button_pressed(encoder_ec11_t *encoder);

/**
 * Set event callback function
 * @param encoder Pointer to encoder instance
 * @param callback Callback function (NULL to disable)
 */
void encoder_ec11_set_callback(encoder_ec11_t *encoder, 
                               void (*callback)(encoder_event_t, int32_t));

/**
 * Poll for encoder events (alternative to interrupt-driven operation)
 * @param encoder Pointer to encoder instance
 * @return Event type if an event occurred
 */
encoder_event_t encoder_ec11_poll(encoder_ec11_t *encoder);

/**
 * Get position change since last call
 * @param encoder Pointer to encoder instance
 * @return Position change (positive = CW, negative = CCW)
 */
int32_t encoder_ec11_get_delta(encoder_ec11_t *encoder);

/**
 * Enable interrupt-driven operation
 * @param encoder Pointer to encoder instance
 * @return HW_OK on success
 */
hw_result_t encoder_ec11_enable_interrupts(encoder_ec11_t *encoder);

/**
 * Disable interrupt-driven operation
 * @param encoder Pointer to encoder instance
 */
void encoder_ec11_disable_interrupts(encoder_ec11_t *encoder);

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * Convert encoder position to degrees (360 degrees = full rotation)
 * @param position Encoder position
 * @param counts_per_rev Encoder counts per revolution
 * @return Angle in degrees
 */
static inline float encoder_position_to_degrees(int32_t position, int32_t counts_per_rev) {
    return (position * 360.0f) / counts_per_rev;
}

/**
 * Convert degrees to encoder position
 * @param degrees Angle in degrees
 * @param counts_per_rev Encoder counts per revolution
 * @return Encoder position
 */
static inline int32_t encoder_degrees_to_position(float degrees, int32_t counts_per_rev) {
    return (int32_t)((degrees * counts_per_rev) / 360.0f);
}

#endif // ENCODER_EC11_H