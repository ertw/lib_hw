#include <stdio.h>
#include "pico.h"
#include "pico/types.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "../lib/lib.h"
#include "../lib/button/button.h"
#include "../lib/rgb_led/ws2812.h"

// Pin definitions
#define BUTTON_PIN 19  // Wukong 2040: Button B = GP19 (active-low)
#define LED_PIN 22     // Wukong 2040: NeoPixels (2x WS2812) on GP22
#define NUM_LEDS 2

// Debug callback for button events
void button_debug_callback(button_event_t event, uint8_t click_count) {
    printf("*** BUTTON CALLBACK: Event=%s, Clicks=%d\n", 
           button_event_to_string(event), click_count);
}

// Repeating timer to wake CPU while using __wfi so debouncing can progress
static repeating_timer_t wfi_timer;
static volatile uint32_t wfi_ticks = 0;
static bool wfi_timer_callback(repeating_timer_t *t) {
    // Increment tick counter; presence of this timer interrupt wakes CPU from WFI
    wfi_ticks++;
    return true; // keep repeating
}

int main() {
    stdio_init_all();
    sleep_ms(200);
    printf("RGB LED demo ready.\n");

    // Initialize button with proper debouncing
    button_t button;
    button_config_t btn_config = {
        .pin = BUTTON_PIN,
        .active_low = true,     // Button connects to ground when pressed
        .pull_up = true,        // Use internal pull-up
        .debounce_ms = 20,      // 20ms debounce time
        .long_press_ms = 1000,  // Not used in this demo
        .multi_click_ms = 400   // Not used in this demo
    };
    
    if (button_init(&button, &btn_config) != HW_OK) {
        printf("Failed to initialize button!\n");
        return -1;
    }
    
    // Set debug callback
    button_set_callback(&button, button_debug_callback);
    printf("Button callback set.\n");
    
    // Enable interrupt-driven button detection
    if (button_enable_interrupts(&button) != HW_OK) {
        printf("Failed to enable button interrupts!\n");
        return -1;
    }
    printf("Button interrupts enabled.\n");

    // Start a 5ms repeating timer to periodically wake CPU from __wfi
    if (!add_repeating_timer_ms(5, wfi_timer_callback, NULL, &wfi_timer)) {
        printf("Failed to start repeating timer!\n");
        return -1;
    }
    printf("WFI wake timer started (5ms).\n");

    // Initialize RGB LED strip
    hw_ws2812_config_t config = {
        .pio = pio0,
        .sm = 0,
        .data_pin = LED_PIN,
        .num_pixels = NUM_LEDS
    };

    hw_ws2812_t ws;
    if (hw_ws2812_init(&ws, &config) != HW_OK) {
        printf("Failed to initialize WS2812 LEDs!\n");
        return -1;
    }

    // Define color palette (using brighter values)
    const struct {
        uint8_t r, g, b;
    } palette[] = {
        {64, 0, 0},   // Red
        {0, 64, 0},   // Green
        {0, 0, 64},   // Blue
        {64, 64, 0},  // Yellow
        {64, 0, 64},  // Magenta
        {0, 64, 64},  // Cyan
        {32, 32, 32}  // White (dimmer)
    };
    const int num_colors = ARRAY_SIZE(palette);
    int color_idx = 0;

    // Test sequence - flash through all colors on startup
    printf("Running LED test sequence...\n");
    for (int i = 0; i < num_colors; i++) {
        printf("Test color %d: R=%d, G=%d, B=%d\n", i,
               palette[i].r, palette[i].g, palette[i].b);
        hw_ws2812_set_all(&ws, palette[i].r, palette[i].g, palette[i].b);
        hw_ws2812_show(&ws);
        sleep_ms(500);
    }
    
    // Clear LEDs
    hw_ws2812_clear(&ws);
    hw_ws2812_show(&ws);
    sleep_ms(200);
    
    // Set initial color
    printf("Setting initial color: R=%d, G=%d, B=%d\n", 
           palette[color_idx].r, palette[color_idx].g, palette[color_idx].b);
    hw_ws2812_set_all(&ws, 
                      palette[color_idx].r,
                      palette[color_idx].g,
                      palette[color_idx].b);
    hw_ws2812_show(&ws);
    printf("LEDs ready. Press button to cycle colors.\n");

    printf("Starting main loop. Initial button state:\n");
    printf("  - Raw GPIO pin %d = %d\n", BUTTON_PIN, gpio_get(BUTTON_PIN));
    printf("  - button_is_pressed() = %d\n", button_is_pressed(&button));
    printf("  - button_get_raw_state() = %d\n", button_get_raw_state(&button));
    
    uint32_t loop_count = 0;
    bool last_gpio_state = gpio_get(BUTTON_PIN);
    
    while (true) {
        // Get raw GPIO state for debugging
        bool gpio_state = gpio_get(BUTTON_PIN);
        bool raw_state = button_get_raw_state(&button);
        bool pressed = button_is_pressed(&button);
        
        // Log GPIO state changes
        if (gpio_state != last_gpio_state) {
            printf(">>> GPIO STATE CHANGE: %d -> %d (pin %d)\n", 
                   last_gpio_state, gpio_state, BUTTON_PIN);
            last_gpio_state = gpio_state;
        }
        
        // Poll button state
        button_event_t event = button_poll(&button);
        
        // Log any non-NONE events
        if (event != BUTTON_EVENT_NONE) {
            printf("[%u] Event: %s, GPIO=%d, Raw=%d, Pressed=%d\n", 
                   loop_count, button_event_to_string(event), 
                   gpio_state, raw_state, pressed);
        }
        
        // Every 1000 loops, show we're alive and timer ticks observed
        if (++loop_count % 1000 == 0) {
            printf("[%u] Alive - GPIO=%d, Raw=%d, Pressed=%d, wfi_ticks=%u\n", 
                   loop_count, gpio_state, raw_state, pressed, wfi_ticks);
        }
        
        // On button click, advance to next color
        if (event == BUTTON_EVENT_CLICK) {
            printf("CLICK DETECTED! Changing color...\n");
            
            // Advance to next color
            color_idx = (color_idx + 1) % num_colors;
            
            // Update LEDs
            hw_ws2812_set_all(&ws,
                             palette[color_idx].r,
                             palette[color_idx].g,
                             palette[color_idx].b);
            hw_ws2812_show(&ws);
            printf("Color index -> %d (R=%d, G=%d, B=%d)\n", color_idx,
                   palette[color_idx].r, palette[color_idx].g, palette[color_idx].b);
        }
        
        // Use WFI (Wait For Interrupt) to sleep until next interrupt
        // This puts the CPU into a low-power state until ANY interrupt occurs
        // (including our button GPIO interrupt)
        
        // Sleep until next interrupt (GPIO edge or repeating timer)
        __wfi();
    }
}