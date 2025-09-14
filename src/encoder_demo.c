/**
 * @file encoder_demo.c
 * @brief EC11 Rotary Encoder Demo with OLED Display using hardware library
 * 
 * Uses an EC11 rotary encoder to control the rotation of shapes
 * displayed on the SH1106 OLED. Push button cycles through shapes.
 * 
 * Encoder connections (using Wukong2040 breakout board):
 * - ENCODER_TRA -> GP26 (Channel A)
 * - ENCODER_TRB -> GP27 (Channel B) 
 * - ENCODER_PUSH -> GP28 (Push button - cycles shapes)
 * - GND -> GND
 * - VCC -> 3.3V
 * 
 * OLED connections:
 * - SDA -> GP6 (I2C1)
 * - SCL -> GP7 (I2C1)
 * - VCC -> VBUS (5V)
 * - GND -> GND
 */

#include "lib.h"
#include <stdio.h>
#include <math.h>

// Pin definitions for EC11 encoder (using accessible pins on Wukong2040)
#define ENCODER_PIN_A    26  // TRA
#define ENCODER_PIN_B    27  // TRB
#define ENCODER_PUSH     28  // Push button (active-low)

// OLED pins
#define OLED_SDA_PIN     6
#define OLED_SCL_PIN     7
#define OLED_ADDR        0x3C

// Shape properties
#define SHAPE_SIZE       20  // Size of the shapes
#define CENTER_X         64  // Center of 128 pixel wide display
#define CENTER_Y         32  // Center of 64 pixel high display

// EC11 has 20 detents per revolution; with X4 decoding that's 80 counts per rev
#define ENCODER_COUNTS_PER_REV 80

// Shape types
typedef enum { 
    SHAPE_SQUARE = 0, 
    SHAPE_TRIANGLE = 1, 
    SHAPE_CIRCLE_X = 2,
    SHAPE_COUNT = 3
} shape_t;

// Global variables
static encoder_ec11_t encoder;
static shape_t current_shape = SHAPE_SQUARE;
static bool shape_changed = false;

// Encoder event callback
void encoder_event_handler(encoder_event_t event, int32_t position) {
    if (event == ENCODER_EVENT_BUTTON_PRESS) {
        // Cycle through shapes on button press
        current_shape = (shape_t)((current_shape + 1) % SHAPE_COUNT);
        shape_changed = true;
    }
}

// Draw a rotated square
static void draw_rotated_square(sh1106_t *display, int cx, int cy, int size, float angle) {
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

// Draw a rotated isosceles triangle
static void draw_rotated_triangle(sh1106_t *display, int cx, int cy, int size, float angle) {
    float half = size / 2.0f;
    float pts[3][2] = {
        { 0.0f, -half}, // top
        {-half,  half}, // bottom-left
        { half,  half}  // bottom-right
    };
    int r[3][2];
    float c = cosf(angle), s = sinf(angle);
    for (int i = 0; i < 3; i++) {
        float x = pts[i][0], y = pts[i][1];
        r[i][0] = (int)(cx + x * c - y * s);
        r[i][1] = (int)(cy + x * s + y * c);
    }
    sh1106_draw_line(display, r[0][0], r[0][1], r[1][0], r[1][1], true);
    sh1106_draw_line(display, r[1][0], r[1][1], r[2][0], r[2][1], true);
    sh1106_draw_line(display, r[2][0], r[2][1], r[0][0], r[0][1], true);
}

// Midpoint circle (outline)
static void draw_circle_outline(sh1106_t *display, int cx, int cy, int rr) {
    int x = rr, y = 0; int err = 0;
    while (x >= y) {
        sh1106_set_pixel(display, cx + x, cy + y, true);
        sh1106_set_pixel(display, cx + y, cy + x, true);
        sh1106_set_pixel(display, cx - y, cy + x, true);
        sh1106_set_pixel(display, cx - x, cy + y, true);
        sh1106_set_pixel(display, cx - x, cy - y, true);
        sh1106_set_pixel(display, cx - y, cy - x, true);
        sh1106_set_pixel(display, cx + y, cy - x, true);
        sh1106_set_pixel(display, cx + x, cy - y, true);
        y++;
        if (err <= 0) { err += 2*y + 1; }
        else { x--; err += 2*(y - x) + 1; }
    }
}

// Circle with rotating cross
static void draw_circle_with_cross(sh1106_t *display, int cx, int cy, int size, float angle) {
    int r = (int)(size / 2.0f);
    draw_circle_outline(display, cx, cy, r);
    float c = cosf(angle), s = sinf(angle);
    // first line
    int x1 = (int)(cx + r * c), y1 = (int)(cy + r * s);
    int x2 = (int)(cx - r * c), y2 = (int)(cy - r * s);
    sh1106_draw_line(display, x1, y1, x2, y2, true);
    // second line (perpendicular)
    float c2 = cosf(angle + (float)M_PI/2.0f), s2 = sinf(angle + (float)M_PI/2.0f);
    int x3 = (int)(cx + r * c2), y3 = (int)(cy + r * s2);
    int x4 = (int)(cx - r * c2), y4 = (int)(cy - r * s2);
    sh1106_draw_line(display, x3, y3, x4, y4, true);
}

int main() {
    stdio_init_all();
    
    // Wait a bit for USB serial to connect (optional)
    sleep_ms(2000);
    
    printf("EC11 Rotary Encoder Demo on Wukong2040 (using hardware library)\n");
    printf("Rotate encoder to spin shapes\n");
    printf("Press encoder button to cycle through shapes\n");
    printf("Encoder pins: A=GP%d, B=GP%d, Push=GP%d\n", 
           ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PUSH);
    
    // Configure and initialize encoder
    encoder_config_t encoder_config = {
        .pin_a = ENCODER_PIN_A,
        .pin_b = ENCODER_PIN_B,
        .pin_button = ENCODER_PUSH,
        .invert_direction = false,
        .debounce_us = 1000,
        .button_debounce_us = 50000,
        .pull_up = true
    };
    
    if (encoder_ec11_init(&encoder, &encoder_config) != HW_OK) {
        printf("Failed to initialize encoder!\n");
        return -1;
    }
    
    // Set encoder limits for one full rotation
    encoder_ec11_set_limits(&encoder, 0, ENCODER_COUNTS_PER_REV - 1, true);
    
    // Set callback for button events
    encoder_ec11_set_callback(&encoder, encoder_event_handler);
    
    // Enable interrupts for encoder
    encoder_ec11_enable_interrupts(&encoder);
    
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
        // Get current encoder position
        int current_position = encoder_ec11_get_position(&encoder);
        
        // Check if encoder position or shape has changed
        if (current_position != last_position || shape_changed) {
            // Clear display buffer
            sh1106_clear(&display);
            
            // Calculate rotation angle (map 0-79 to 0-2π)
            float angle = (current_position * 2.0f * M_PI) / ENCODER_COUNTS_PER_REV;
            
            // Draw the selected shape
            switch (current_shape) {
                case SHAPE_SQUARE:
                    draw_rotated_square(&display, CENTER_X, CENTER_Y, SHAPE_SIZE, angle);
                    break;
                case SHAPE_TRIANGLE:
                    draw_rotated_triangle(&display, CENTER_X, CENTER_Y, SHAPE_SIZE, angle);
                    break;
                case SHAPE_CIRCLE_X:
                    draw_circle_with_cross(&display, CENTER_X, CENTER_Y, SHAPE_SIZE, angle);
                    break;
            }
            
            // Draw position/shape indicator at top
            const char *shape_name = (current_shape == SHAPE_SQUARE) ? "Square" :
                                     (current_shape == SHAPE_TRIANGLE) ? "Triangle" : "Circle+Cross";
            snprintf(status, sizeof(status), "%s | %02d/%d", shape_name, current_position, ENCODER_COUNTS_PER_REV);
            sh1106_draw_string(&display, 0, 0, status);
            
            // Draw angle at bottom
            int degrees = (int)((angle * 180.0f) / M_PI);
            snprintf(status, sizeof(status), "Angle: %03d deg", degrees);
            sh1106_draw_string(&display, 0, 56, status);
            
            // Update display
            sh1106_update(&display);
            
            // Print to serial for debugging
            printf("Shape:%d Position:%d Angle:%d deg\n", (int)current_shape, current_position, degrees);
            
            last_position = current_position;
            shape_changed = false;
        }
        
        // Small delay to reduce CPU usage
        sleep_ms(10);
    }
    
    return 0;
}
