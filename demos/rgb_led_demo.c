#include <stdio.h>
#include "../lib/lib.h"

// Pin definitions
#define BUTTON_PIN 19  // Wukong 2040: Button B = GP19 (active-low)
#define LED_PIN 22     // Wukong 2040: NeoPixels (2x WS2812) on GP22
#define NUM_LEDS 2

int main() {
    stdio_init_all();
    sleep_ms(200);
    printf("RGB LED demo ready.\n");

    // Initialize button (active-low with pull-up)
    hw_gpio_init_input_pullup(BUTTON_PIN);

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

    while (true) {
        // Wait for button press (active-low)
        while (gpio_get(BUTTON_PIN) != 0) {
            sleep_ms(1);
        }

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

        // Wait for release and debounce
        while (gpio_get(BUTTON_PIN) == 0) {
            sleep_ms(1);
        }
        sleep_ms(20);
    }
}