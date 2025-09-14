/**
 * @file stepper_28byj48.h
 * @brief Driver for 28BYJ-48 stepper motor with ULN2003 driver board
 * 
 * This driver provides control for the popular 28BYJ-48 5V stepper motor
 * commonly used with Arduino and Raspberry Pi projects. The motor has a
 * 64:1 gear ratio and requires 512 full steps or 4096 half steps per revolution.
 */

#ifndef STEPPER_28BYJ48_H
#define STEPPER_28BYJ48_H

#include "../lib.h"

// =============================================================================
// Configuration
// =============================================================================

/** Steps per revolution for 28BYJ-48 motor */
#define STEPPER_28BYJ48_STEPS_PER_REV_FULL 512   ///< Full stepping mode
#define STEPPER_28BYJ48_STEPS_PER_REV_HALF 4096  ///< Half stepping mode

/** Default step delay in microseconds */
#define STEPPER_28BYJ48_DEFAULT_STEP_DELAY_US 2500

/** Maximum recommended speed (steps per second) */
#define STEPPER_28BYJ48_MAX_SPEED_SPS 500

// =============================================================================
// Type Definitions
// =============================================================================

/** Stepping mode */
typedef enum {
    STEPPER_MODE_FULL_STEP,     ///< Full step mode (4 steps per cycle)
    STEPPER_MODE_HALF_STEP,     ///< Half step mode (8 steps per cycle)
    STEPPER_MODE_WAVE_DRIVE,    ///< Wave drive mode (4 steps, single coil)
} stepper_mode_t;

/** Stepper motor state */
typedef enum {
    STEPPER_STATE_IDLE,         ///< Motor idle, coils off
    STEPPER_STATE_HOLDING,      ///< Motor holding position, coils energized
    STEPPER_STATE_RUNNING,      ///< Motor running
} stepper_state_t;

/** Stepper motor configuration */
typedef struct {
    uint in1_pin;               ///< IN1 pin (Blue wire on motor)
    uint in2_pin;               ///< IN2 pin (Pink wire on motor)
    uint in3_pin;               ///< IN3 pin (Yellow wire on motor)
    uint in4_pin;               ///< IN4 pin (Orange wire on motor)
    stepper_mode_t mode;        ///< Stepping mode
    uint32_t step_delay_us;     ///< Delay between steps in microseconds
} stepper_config_t;

/** Stepper motor instance */
typedef struct {
    stepper_config_t config;    ///< Motor configuration
    uint8_t current_step;       ///< Current step in sequence (0-3 or 0-7)
    int32_t position;           ///< Current position in steps from origin
    stepper_state_t state;      ///< Current motor state
    absolute_time_t next_step_time; ///< Time for next step
    int32_t target_position;    ///< Target position for movement
    bool continuous_mode;       ///< True for continuous rotation
    hw_direction_t direction;   ///< Current direction
} stepper_28byj48_t;

// =============================================================================
// Step Sequences
// =============================================================================

/** Full step sequence (4 steps) */
static const uint8_t STEPPER_FULLSTEP_SEQUENCE[4] = {
    0b0011,  // Step 0: IN1+IN2
    0b0110,  // Step 1: IN2+IN3
    0b1100,  // Step 2: IN3+IN4
    0b1001,  // Step 3: IN4+IN1
};

/** Half step sequence (8 steps) */
static const uint8_t STEPPER_HALFSTEP_SEQUENCE[8] = {
    0b0001,  // Step 0: IN1
    0b0011,  // Step 1: IN1+IN2
    0b0010,  // Step 2: IN2
    0b0110,  // Step 3: IN2+IN3
    0b0100,  // Step 4: IN3
    0b1100,  // Step 5: IN3+IN4
    0b1000,  // Step 6: IN4
    0b1001,  // Step 7: IN4+IN1
};

/** Wave drive sequence (4 steps, single coil) */
static const uint8_t STEPPER_WAVE_SEQUENCE[4] = {
    0b0001,  // Step 0: IN1
    0b0010,  // Step 1: IN2
    0b0100,  // Step 2: IN3
    0b1000,  // Step 3: IN4
};

// =============================================================================
// Function Prototypes
// =============================================================================

/**
 * Initialize stepper motor
 * @param motor Pointer to motor instance
 * @param config Pointer to configuration
 * @return HW_OK on success, error code otherwise
 */
hw_result_t stepper_28byj48_init(stepper_28byj48_t *motor, const stepper_config_t *config);

/**
 * Drive motor coils with specified pattern
 * @param motor Pointer to motor instance
 * @param pattern 4-bit pattern for IN1-IN4
 */
void stepper_28byj48_drive_pattern(stepper_28byj48_t *motor, uint8_t pattern);

/**
 * Turn off all motor coils
 * @param motor Pointer to motor instance
 */
void stepper_28byj48_coils_off(stepper_28byj48_t *motor);

/**
 * Perform a single step
 * @param motor Pointer to motor instance
 * @param direction Direction to step (DIR_CW or DIR_CCW)
 * @return HW_OK if stepped, HW_BUSY if not time yet
 */
hw_result_t stepper_28byj48_step(stepper_28byj48_t *motor, hw_direction_t direction);

/**
 * Step motor if enough time has elapsed
 * @param motor Pointer to motor instance
 * @return true if step was performed, false otherwise
 */
bool stepper_28byj48_step_if_ready(stepper_28byj48_t *motor);

/**
 * Move motor to absolute position
 * @param motor Pointer to motor instance
 * @param position Target position in steps
 */
void stepper_28byj48_move_to(stepper_28byj48_t *motor, int32_t position);

/**
 * Move motor by relative number of steps
 * @param motor Pointer to motor instance
 * @param steps Number of steps (positive = CW, negative = CCW)
 */
void stepper_28byj48_move_by(stepper_28byj48_t *motor, int32_t steps);

/**
 * Start continuous rotation
 * @param motor Pointer to motor instance
 * @param direction Direction to rotate
 */
void stepper_28byj48_run(stepper_28byj48_t *motor, hw_direction_t direction);

/**
 * Stop motor movement
 * @param motor Pointer to motor instance
 * @param hold If true, keep coils energized to hold position
 */
void stepper_28byj48_stop(stepper_28byj48_t *motor, bool hold);

/**
 * Check if motor is moving
 * @param motor Pointer to motor instance
 * @return true if motor is moving
 */
bool stepper_28byj48_is_moving(stepper_28byj48_t *motor);

/**
 * Get current position
 * @param motor Pointer to motor instance
 * @return Current position in steps
 */
int32_t stepper_28byj48_get_position(stepper_28byj48_t *motor);

/**
 * Reset position to zero
 * @param motor Pointer to motor instance
 */
void stepper_28byj48_reset_position(stepper_28byj48_t *motor);

/**
 * Set motor speed
 * @param motor Pointer to motor instance
 * @param steps_per_second Desired speed in steps per second
 * @return HW_OK on success, HW_INVALID_PARAM if speed too high
 */
hw_result_t stepper_28byj48_set_speed(stepper_28byj48_t *motor, uint16_t steps_per_second);

/**
 * Set stepping mode
 * @param motor Pointer to motor instance
 * @param mode New stepping mode
 */
void stepper_28byj48_set_mode(stepper_28byj48_t *motor, stepper_mode_t mode);

/**
 * Convert degrees to steps based on current mode
 * @param motor Pointer to motor instance
 * @param degrees Angle in degrees
 * @return Number of steps
 */
int32_t stepper_28byj48_degrees_to_steps(stepper_28byj48_t *motor, float degrees);

/**
 * Convert steps to degrees based on current mode
 * @param motor Pointer to motor instance
 * @param steps Number of steps
 * @return Angle in degrees
 */
float stepper_28byj48_steps_to_degrees(stepper_28byj48_t *motor, int32_t steps);

#endif // STEPPER_28BYJ48_H