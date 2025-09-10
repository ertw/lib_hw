/**
 * EC11 Rotary Encoder Demo with OLED Display
 * 
 * Uses an EC11 rotary encoder to control the rotation of a square
 * displayed on the SH1106 OLED.
 * 
 * Encoder connections (using Wukong2040 breakout board):
 * - ENCODER_TRA -> GP26 (Channel A)
 * - ENCODER_TRB -> GP27 (Channel B) 
 * - ENCODER_PUSH -> GP28 (Push button - unused for now)
 * - GND -> GND
 * - VCC -> 3.3V
 * 
 * OLED connections (as before):
 * - SDA -> GP6 (I2C1)
 * - SCL -> GP7 (I2C1)
 * - VCC -> VBUS (5V)
 * - GND -> GND
 */

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sh1106.h"

// Pin definitions for EC11 encoder (using accessible pins on Wukong2040)
#define ENCODER_PIN_A    26  // TRA
#define ENCODER_PIN_B    27  // TRB
#define ENCODER_PUSH     28  // Push button (unused for now)

// OLED pins
#define OLED_SDA_PIN     6
#define OLED_SCL_PIN     7
#define OLED_ADDR        0x3C

// Square properties
#define SQUARE_SIZE      20  // Size of the square
#define CENTER_X         64  // Center of 128 pixel wide display
#define CENTER_Y         32  // Center of 64 pixel high display

// EC11 has 20 detents per revolution; with X4 decoding that's 80 counts per rev
#define ENCODER_COUNTS_PER_REV 80

// Global variables for encoder state
static volatile int encoder_position = 0;
static volatile uint8_t last_encoder_state = 0;

// Interrupt handler for encoder
void encoder_callback(uint gpio, uint32_t events) {
    // Read current state of both pins
    uint8_t pin_a = gpio_get(ENCODER_PIN_A);
    uint8_t pin_b = gpio_get(ENCODER_PIN_B);
    uint8_t current_state = (pin_a << 1) | pin_b;
    
    // Gray code state machine for quadrature decoding
    // This determines direction based on state transitions
    static const int8_t encoder_table[] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };
    
    uint8_t index = (last_encoder_state << 2) | current_state;
    int8_t direction = encoder_table[index];
    
    if (direction != 0) {
        encoder_position += direction;
        
        // Wrap around at full rotation (80 counts per rev with X4 decoding)
        if (encoder_position >= ENCODER_COUNTS_PER_REV) {
            encoder_position = 0;
        } else if (encoder_position < 0) {
            encoder_position = ENCODER_COUNTS_PER_REV - 1;
        }
    }
    
    last_encoder_state = current_state;
}

// Initialize encoder pins and interrupts
void encoder_init(void) {
    // Initialize encoder pins with pull-ups
    gpio_init(ENCODER_PIN_A);
    gpio_init(ENCODER_PIN_B);
    gpio_init(ENCODER_PUSH);
    
    gpio_set_dir(ENCODER_PIN_A, GPIO_IN);
    gpio_set_dir(ENCODER_PIN_B, GPIO_IN);
    gpio_set_dir(ENCODER_PUSH, GPIO_IN);
    
    gpio_pull_up(ENCODER_PIN_A);
    gpio_pull_up(ENCODER_PIN_B);
    gpio_pull_up(ENCODER_PUSH);
    
    // Read initial state
    last_encoder_state = (gpio_get(ENCODER_PIN_A) << 1) | gpio_get(ENCODER_PIN_B);
    
    // Enable interrupts on both encoder pins for any edge
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, 
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
        true, &encoder_callback);
    gpio_set_irq_enabled(ENCODER_PIN_B, 
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
        true);
}

// Draw a rotated square
void draw_rotated_square(sh1106_t *display, int cx, int cy, int size, float angle) {
    // Calculate half size for convenience
    float half = size / 2.0f;
    
    // Calculate corner points relative to center
    float corners[4][2] = {
        {-half, -half},  // Top-left
        { half, -half},  // Top-right
        { half,  half},  // Bottom-right
        {-half,  half}   // Bottom-left
    };
    
    // Rotate and translate each corner
    int rotated[4][2];
    for (int i = 0; i < 4; i++) {
        float x = corners[i][0];
        float y = corners[i][1];
        
        // Apply rotation
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);
        rotated[i][0] = (int)(cx + x * cos_a - y * sin_a);
        rotated[i][1] = (int)(cy + x * sin_a + y * cos_a);
    }
    
    // Draw lines between corners to form the square
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        sh1106_draw_line(display, 
            rotated[i][0], rotated[i][1],
            rotated[next][0], rotated[next][1], 
            true);
    }
    
    // Draw a small dot at the center
    sh1106_set_pixel(display, cx, cy, true);
}

int main() {
    stdio_init_all();
    
    // Wait a bit for USB serial to connect (optional)
    sleep_ms(2000);
    
    printf("EC11 Rotary Encoder Demo on Wukong2040\n");
    printf("Rotate encoder to spin the square\n");
    printf("Encoder pins: A=GP%d, B=GP%d, Push=GP%d\n", 
           ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PUSH);
    
    // Initialize encoder
    encoder_init();
    printf("Encoder initialized\n");
    
    // Initialize OLED display
    sh1106_t display;
    if (!sh1106_init(&display, i2c1, OLED_ADDR, OLED_SDA_PIN, OLED_SCL_PIN)) {
        printf("Failed to initialize OLED display!\n");
        while (1) {
            tight_loop_contents();
        }
    }
    printf("OLED initialized\n");
    
    // Main loop
    int last_position = -1;
    char status[32];
    
    while (true) {
        // Check if encoder position has changed
        int current_position = encoder_position;
        
        if (current_position != last_position) {
            // Clear display buffer
            sh1106_clear(&display);
            
            // Calculate rotation angle (map 0-79 to 0-2π)
            float angle = (current_position * 2.0f * M_PI) / ENCODER_COUNTS_PER_REV;
            
            // Draw the rotated square
            draw_rotated_square(&display, CENTER_X, CENTER_Y, SQUARE_SIZE, angle);
            
            // Draw position indicator at top
            snprintf(status, sizeof(status), "Count: %02d/%d", 
                     current_position, ENCODER_COUNTS_PER_REV);
            sh1106_draw_string(&display, 0, 0, status);
            
            // Draw angle at bottom
            int degrees = (int)((angle * 180.0f) / M_PI);
            snprintf(status, sizeof(status), "Angle: %03d deg", degrees);
            sh1106_draw_string(&display, 0, 56, status);
            
            // Update display
            sh1106_update(&display);
            
            // Print to serial for debugging
            printf("Position: %d, Angle: %d degrees\n", current_position, degrees);
            
            last_position = current_position;
        }
        
        // Small delay to reduce CPU usage
        sleep_ms(10);
    }
    
    return 0;
}
