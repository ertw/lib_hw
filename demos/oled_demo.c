/**
 * @file oled_demo.c
 * @brief OLED display demo using the hardware library
 * 
 * Demonstrates SH1106 OLED display capabilities including text,
 * graphics primitives, and animations.
 */

#include "lib.h"
#include <stdio.h>
#include <math.h>

// I2C Configuration - UPDATED based on testing
// GP6/GP7 work with i2c1, NOT i2c0!
#define I2C_SDA_PIN 6   // Pin 9 physical
#define I2C_SCL_PIN 7   // Pin 10 physical
#define I2C_PORT i2c1   // MUST use i2c1 for these pins
#define OLED_ADDR 0x3C  // Your display's actual address

// Animation variables
static int animation_frame = 0;

// Function to draw a simple animation
void draw_animation(sh1106_t *display, int frame) {
    // Clear previous animation area
    sh1106_draw_rect(display, 80, 16, 48, 48, false);
    
    // Draw rotating box
    int cx = 104;  // Center X
    int cy = 40;   // Center Y
    int size = 15;
    
    // Calculate rotation angle
    float angle = frame * 0.1f;
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    
    // Box corners relative to center
    int corners[4][2] = {
        {-size, -size},
        {size, -size},
        {size, size},
        {-size, size}
    };
    
    // Rotate and draw lines between corners
    int rotated[4][2];
    for (int i = 0; i < 4; i++) {
        rotated[i][0] = cx + (int)(corners[i][0] * cos_a - corners[i][1] * sin_a);
        rotated[i][1] = cy + (int)(corners[i][0] * sin_a + corners[i][1] * cos_a);
    }
    
    // Draw the box
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        sh1106_draw_line(display, 
                         rotated[i][0], rotated[i][1],
                         rotated[next][0], rotated[next][1], true);
    }
    
    // Draw a bouncing ball
    int ball_x = 90 + (int)(10 * sinf(frame * 0.2f));
    int ball_y = 40 + (int)(15 * fabsf(sinf(frame * 0.15f)));
    
    // Draw ball (small filled circle approximation)
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            if (dx*dx + dy*dy <= 4) {
                sh1106_set_pixel(display, ball_x + dx, ball_y + dy, true);
            }
        }
    }
}

// Function to display system info
void display_system_info(sh1106_t *display) {
    char buffer[32];
    
    // Title
    sh1106_draw_string(display, 20, 0, "Pico 2 W OLED");
    
    // Draw a separator line
    sh1106_draw_line(display, 0, 10, 127, 10, true);
    
    // Display some info
    sh1106_draw_string(display, 0, 16, "SH1106 128x64");
    sh1106_draw_string(display, 0, 26, "I2C @ 400kHz");
    
    // Display pins info
    snprintf(buffer, sizeof(buffer), "SDA: GP%d", I2C_SDA_PIN);
    sh1106_draw_string(display, 0, 36, buffer);
    snprintf(buffer, sizeof(buffer), "SCL: GP%d", I2C_SCL_PIN);
    sh1106_draw_string(display, 0, 46, buffer);
    
    // Status
    sh1106_draw_string(display, 0, 56, "Status: OK!");
}

int main() {
    // Initialize stdio for USB serial output
    stdio_init_all();
    
    // Wait a bit for USB serial to connect
    sleep_ms(2000);
    
    printf("SH1106 OLED Demo\n");
    printf("Configuration:\n");
    printf("  I2C Port: i2c1 (IMPORTANT!)\n");
    printf("  SDA: GP%d (Pin 9)\n", I2C_SDA_PIN);
    printf("  SCL: GP%d (Pin 10)\n", I2C_SCL_PIN);
    printf("  Address: 0x%02X\n", OLED_ADDR);
    printf("  Power: 5V (VBUS Pin 40)\n\n");
    
    // Create display instance
    sh1106_t display;
    
    // Initialize the display with i2c1 and correct address
    if (sh1106_init(&display, I2C_PORT, OLED_ADDR, I2C_SDA_PIN, I2C_SCL_PIN) != HW_OK) {
        printf("Failed to initialize OLED display!\n");
        printf("Check connections:\n");
        printf("  VCC -> 5V (VBUS Pin 40)\n");
        printf("  GND -> GND\n");
        printf("  SDA -> GP%d (Pin 9)\n", I2C_SDA_PIN);
        printf("  SCL -> GP%d (Pin 10)\n", I2C_SCL_PIN);
        
        // Pico W boards don't have a built-in LED on a GPIO pin
        // Just loop forever with an error state
        while (true) {
            sleep_ms(1000);
        }
    }
    
    printf("OLED initialized successfully!\n");
    
    // Quick test: Turn all pixels on briefly to verify display works
    printf("Testing display - all pixels ON for 1 second...\n");
    sh1106_entire_display(&display, true);  // Turn all pixels on
    sleep_ms(1000);
    sh1106_entire_display(&display, false); // Back to normal mode
    
    // Test inverted display
    printf("Testing inverted display...\n");
    sh1106_clear(&display);
    sh1106_draw_string(&display, 10, 28, "DISPLAY TEST");
    sh1106_update(&display);
    sleep_ms(1000);
    sh1106_invert(&display, true);
    sleep_ms(1000);
    sh1106_invert(&display, false);
    sleep_ms(500);
    
    printf("Starting demo sequence...\n\n");
    
    // Demo sequence
    int demo_state = 0;
    absolute_time_t next_state_change = make_timeout_time_ms(3000);
    
    while (true) {
        switch (demo_state) {
            case 0:
                // Welcome screen
                sh1106_clear(&display);
                sh1106_draw_string(&display, 25, 20, "Pico 2 W");
                sh1106_draw_string(&display, 15, 30, "OLED Demo");
                sh1106_draw_string(&display, 20, 45, "SH1106 I2C");
                sh1106_update(&display);
                
                if (time_reached(next_state_change)) {
                    demo_state = 1;
                    next_state_change = make_timeout_time_ms(5000);
                }
                break;
                
            case 1:
                // System info screen
                sh1106_clear(&display);
                display_system_info(&display);
                sh1106_update(&display);
                
                if (time_reached(next_state_change)) {
                    demo_state = 2;
                    next_state_change = make_timeout_time_ms(100);  // Faster update for animation
                    animation_frame = 0;
                }
                break;
                
            case 2:
                // Graphics demo
                if (animation_frame == 0) {
                    sh1106_clear(&display);
                    sh1106_draw_string(&display, 0, 0, "Graphics Demo");
                    sh1106_draw_line(&display, 0, 10, 127, 10, true);
                    
                    // Draw some static shapes
                    sh1106_draw_rect(&display, 5, 20, 20, 20, false);  // Empty rectangle
                    sh1106_draw_rect(&display, 30, 20, 20, 20, true);  // Filled rectangle
                    
                    // Draw triangle
                    sh1106_draw_line(&display, 15, 50, 5, 60, true);
                    sh1106_draw_line(&display, 5, 60, 25, 60, true);
                    sh1106_draw_line(&display, 25, 60, 15, 50, true);
                    
                    // Draw cross
                    sh1106_draw_line(&display, 35, 45, 45, 55, true);
                    sh1106_draw_line(&display, 45, 45, 35, 55, true);
                }
                
                // Animate
                draw_animation(&display, animation_frame);
                sh1106_update(&display);
                
                animation_frame++;
                if (animation_frame > 300) {
                    demo_state = 3;
                    next_state_change = make_timeout_time_ms(3000);
                }
                
                if (time_reached(next_state_change)) {
                    next_state_change = make_timeout_time_ms(50);  // Animation frame rate
                }
                break;
                
            case 3:
                // Text scrolling demo
                sh1106_clear(&display);
                sh1106_draw_string(&display, 10, 0, "Text Demo:");
                sh1106_draw_line(&display, 0, 10, 127, 10, true);
                
                sh1106_draw_string(&display, 0, 16, "ABCDEFGHIJKLM");
                sh1106_draw_string(&display, 0, 26, "NOPQRSTUVWXYZ");
                sh1106_draw_string(&display, 0, 36, "0123456789");
                sh1106_draw_string(&display, 0, 46, "!@#$%^&*()");
                sh1106_draw_string(&display, 0, 56, "Hello World!");
                sh1106_update(&display);
                
                if (time_reached(next_state_change)) {
                    demo_state = 4;
                    next_state_change = make_timeout_time_ms(3000);
                }
                break;
                
            case 4:
                // Contrast test
                sh1106_clear(&display);
                sh1106_draw_string(&display, 10, 20, "Contrast Test");
                sh1106_update(&display);
                
                // Cycle through contrast levels
                for (int i = 0; i < 256; i += 32) {
                    sh1106_set_contrast(&display, i);
                    sleep_ms(300);
                }
                sh1106_set_contrast(&display, 0x7F);  // Reset to default
                
                demo_state = 0;  // Loop back to start
                next_state_change = make_timeout_time_ms(1000);
                break;
                
            default:
                demo_state = 0;
                break;
        }
        
        sleep_ms(10);  // Small delay to prevent busy-waiting
    }
    
    return 0;
}
