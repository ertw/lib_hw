/**
 * @file main.c
 * @brief Stepper motor demo using the hardware library
 * 
 * Controls a 28BYJ-48 stepper motor with push buttons.
 */

#include "lib.h"
#include <stdio.h>

// Motor pins (connected to ULN2003 driver)
#define PIN_IN1 2
#define PIN_IN2 3
#define PIN_IN3 4
#define PIN_IN4 5

// Button pins
#define BUTTON_CW 18   // Button for clockwise rotation
#define BUTTON_CCW 19  // Button for counter-clockwise rotation

int main() {
    stdio_init_all();
    
    // Configure stepper motor
    stepper_config_t motor_config = {
        .in1_pin = PIN_IN1,
        .in2_pin = PIN_IN2,
        .in3_pin = PIN_IN3,
        .in4_pin = PIN_IN4,
        .mode = STEPPER_MODE_HALF_STEP,
        .step_delay_us = 2500
    };
    
    stepper_28byj48_t motor;
    if (stepper_28byj48_init(&motor, &motor_config) != HW_OK) {
        printf("Failed to initialize stepper motor\n");
        return -1;
    }
    
    // Initialize button pins with pull-up resistors
    hw_gpio_init_input_pullup(BUTTON_CW);
    hw_gpio_init_input_pullup(BUTTON_CCW);
    
    printf("Stepper Motor Control (using hardware library)\n");
    printf("Press button on GP%d for clockwise rotation\n", BUTTON_CW);
    printf("Press button on GP%d for counter-clockwise rotation\n", BUTTON_CCW);
    printf("Both buttons: Stop\n");
    
    while (true) {
        // Read button states (buttons are active low with pull-up)
        bool cw_pressed = !gpio_get(BUTTON_CW);
        bool ccw_pressed = !gpio_get(BUTTON_CCW);
        
        if (cw_pressed && !ccw_pressed) {
            // Clockwise rotation
            stepper_28byj48_run(&motor, DIR_CW);
        } else if (ccw_pressed && !cw_pressed) {
            // Counter-clockwise rotation
            stepper_28byj48_run(&motor, DIR_CCW);
        } else {
            // No button pressed or both pressed - stop motor
            stepper_28byj48_stop(&motor, false);  // Don't hold position
        }
        
        // Step the motor if it's running
        stepper_28byj48_step_if_ready(&motor);
        
        // Small delay to prevent busy-waiting
        hw_sleep_us(100);
    }
}
