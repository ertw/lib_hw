/**
 * @file stepper_28byj48.c
 * @brief Implementation of 28BYJ-48 stepper motor driver
 */

#include "stepper_28byj48.h"

// =============================================================================
// Private Functions
// =============================================================================

/**
 * Get step sequence based on mode
 */
static const uint8_t* get_sequence(stepper_mode_t mode, uint8_t *length) {
    switch (mode) {
        case STEPPER_MODE_FULL_STEP:
            *length = 4;
            return STEPPER_FULLSTEP_SEQUENCE;
        case STEPPER_MODE_HALF_STEP:
            *length = 8;
            return STEPPER_HALFSTEP_SEQUENCE;
        case STEPPER_MODE_WAVE_DRIVE:
            *length = 4;
            return STEPPER_WAVE_SEQUENCE;
        default:
            *length = 8;
            return STEPPER_HALFSTEP_SEQUENCE;
    }
}

/**
 * Get steps per revolution based on mode
 */
static int32_t get_steps_per_rev(stepper_mode_t mode) {
    switch (mode) {
        case STEPPER_MODE_FULL_STEP:
        case STEPPER_MODE_WAVE_DRIVE:
            return STEPPER_28BYJ48_STEPS_PER_REV_FULL;
        case STEPPER_MODE_HALF_STEP:
        default:
            return STEPPER_28BYJ48_STEPS_PER_REV_HALF;
    }
}

// =============================================================================
// Public Functions
// =============================================================================

hw_result_t stepper_28byj48_init(stepper_28byj48_t *motor, const stepper_config_t *config) {
    if (!motor || !config) {
        return HW_INVALID_PARAM;
    }
    
    // Copy configuration
    motor->config = *config;
    
    // Initialize GPIO pins
    hw_gpio_init_output_val(config->in1_pin, false);
    hw_gpio_init_output_val(config->in2_pin, false);
    hw_gpio_init_output_val(config->in3_pin, false);
    hw_gpio_init_output_val(config->in4_pin, false);
    
    // Initialize state
    motor->current_step = 0;
    motor->position = 0;
    motor->state = STEPPER_STATE_IDLE;
    motor->next_step_time = get_absolute_time();
    motor->target_position = 0;
    motor->continuous_mode = false;
    motor->direction = DIR_CW;
    
    // Set default step delay if not specified
    if (motor->config.step_delay_us == 0) {
        motor->config.step_delay_us = STEPPER_28BYJ48_DEFAULT_STEP_DELAY_US;
    }
    
    return HW_OK;
}

void stepper_28byj48_drive_pattern(stepper_28byj48_t *motor, uint8_t pattern) {
    gpio_put(motor->config.in1_pin, pattern & 0x01);
    gpio_put(motor->config.in2_pin, (pattern >> 1) & 0x01);
    gpio_put(motor->config.in3_pin, (pattern >> 2) & 0x01);
    gpio_put(motor->config.in4_pin, (pattern >> 3) & 0x01);
}

void stepper_28byj48_coils_off(stepper_28byj48_t *motor) {
    stepper_28byj48_drive_pattern(motor, 0);
    motor->state = STEPPER_STATE_IDLE;
}

hw_result_t stepper_28byj48_step(stepper_28byj48_t *motor, hw_direction_t direction) {
    // Check if enough time has elapsed
    if (!time_reached(motor->next_step_time)) {
        return HW_BUSY;
    }
    
    uint8_t seq_length;
    const uint8_t *sequence = get_sequence(motor->config.mode, &seq_length);
    
    // Update step index
    if (direction == DIR_CW) {
        motor->current_step = (motor->current_step + 1) % seq_length;
        motor->position++;
    } else {
        motor->current_step = (motor->current_step + seq_length - 1) % seq_length;
        motor->position--;
    }
    
    // Drive the motor
    stepper_28byj48_drive_pattern(motor, sequence[motor->current_step]);
    
    // Update timing
    motor->next_step_time = make_timeout_time_us(motor->config.step_delay_us);
    motor->state = STEPPER_STATE_RUNNING;
    motor->direction = direction;
    
    return HW_OK;
}

bool stepper_28byj48_step_if_ready(stepper_28byj48_t *motor) {
    if (!stepper_28byj48_is_moving(motor)) {
        return false;
    }
    
    // Determine direction for target position
    hw_direction_t dir = DIR_CW;
    if (!motor->continuous_mode) {
        if (motor->position > motor->target_position) {
            dir = DIR_CCW;
        } else if (motor->position == motor->target_position) {
            // Reached target
            stepper_28byj48_stop(motor, true);
            return false;
        }
    } else {
        dir = motor->direction;
    }
    
    return (stepper_28byj48_step(motor, dir) == HW_OK);
}

void stepper_28byj48_move_to(stepper_28byj48_t *motor, int32_t position) {
    motor->target_position = position;
    motor->continuous_mode = false;
    motor->state = STEPPER_STATE_RUNNING;
}

void stepper_28byj48_move_by(stepper_28byj48_t *motor, int32_t steps) {
    stepper_28byj48_move_to(motor, motor->position + steps);
}

void stepper_28byj48_run(stepper_28byj48_t *motor, hw_direction_t direction) {
    motor->direction = direction;
    motor->continuous_mode = true;
    motor->state = STEPPER_STATE_RUNNING;
}

void stepper_28byj48_stop(stepper_28byj48_t *motor, bool hold) {
    motor->continuous_mode = false;
    motor->target_position = motor->position;
    
    if (hold) {
        motor->state = STEPPER_STATE_HOLDING;
    } else {
        stepper_28byj48_coils_off(motor);
    }
}

bool stepper_28byj48_is_moving(stepper_28byj48_t *motor) {
    if (motor->continuous_mode) {
        return motor->state == STEPPER_STATE_RUNNING;
    }
    return motor->position != motor->target_position;
}

int32_t stepper_28byj48_get_position(stepper_28byj48_t *motor) {
    return motor->position;
}

void stepper_28byj48_reset_position(stepper_28byj48_t *motor) {
    motor->position = 0;
    motor->target_position = 0;
}

hw_result_t stepper_28byj48_set_speed(stepper_28byj48_t *motor, uint16_t steps_per_second) {
    if (steps_per_second > STEPPER_28BYJ48_MAX_SPEED_SPS) {
        return HW_INVALID_PARAM;
    }
    
    if (steps_per_second == 0) {
        motor->config.step_delay_us = STEPPER_28BYJ48_DEFAULT_STEP_DELAY_US;
    } else {
        motor->config.step_delay_us = 1000000 / steps_per_second;
    }
    
    return HW_OK;
}

void stepper_28byj48_set_mode(stepper_28byj48_t *motor, stepper_mode_t mode) {
    // Convert position to maintain physical angle
    int32_t old_steps_per_rev = get_steps_per_rev(motor->config.mode);
    int32_t new_steps_per_rev = get_steps_per_rev(mode);
    
    motor->position = (motor->position * new_steps_per_rev) / old_steps_per_rev;
    motor->target_position = (motor->target_position * new_steps_per_rev) / old_steps_per_rev;
    
    motor->config.mode = mode;
    motor->current_step = 0;
}

int32_t stepper_28byj48_degrees_to_steps(stepper_28byj48_t *motor, float degrees) {
    int32_t steps_per_rev = get_steps_per_rev(motor->config.mode);
    return (int32_t)((degrees / 360.0f) * steps_per_rev);
}

float stepper_28byj48_steps_to_degrees(stepper_28byj48_t *motor, int32_t steps) {
    int32_t steps_per_rev = get_steps_per_rev(motor->config.mode);
    return (steps * 360.0f) / steps_per_rev;
}