
#include "pico/stdlib.h"
#include <stdio.h>

#ifndef PIN_IN1
#define PIN_IN1 2
#endif
#ifndef PIN_IN2
#define PIN_IN2 3
#endif
#ifndef PIN_IN3
#define PIN_IN3 4
#endif
#ifndef PIN_IN4
#define PIN_IN4 5
#endif

// Button pins
#define BUTTON_CW 18   // Button for clockwise rotation
#define BUTTON_CCW 19  // Button for counter-clockwise rotation

static const uint8_t HALFSTEP[8] = {
    0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001
};

#ifndef STEP_US
#define STEP_US 2500
#endif

static inline void drive_mask(uint8_t m) {
    gpio_put(PIN_IN1, m & 0x1);
    gpio_put(PIN_IN2, (m >> 1) & 0x1);
    gpio_put(PIN_IN3, (m >> 2) & 0x1);
    gpio_put(PIN_IN4, (m >> 3) & 0x1);
}

static void coils_off(void) { drive_mask(0); }

int main() {
    stdio_init_all();
    
    // Initialize motor control pins
    const uint8_t pins[4] = {PIN_IN1, PIN_IN2, PIN_IN3, PIN_IN4};
    for (int i = 0; i < 4; ++i) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
        gpio_put(pins[i], 0);
    }
    
    // Initialize button pins with pull-up resistors
    gpio_init(BUTTON_CW);
    gpio_set_dir(BUTTON_CW, GPIO_IN);
    gpio_pull_up(BUTTON_CW);
    
    gpio_init(BUTTON_CCW);
    gpio_set_dir(BUTTON_CCW, GPIO_IN);
    gpio_pull_up(BUTTON_CCW);
    
    int idx = 0;
    absolute_time_t next_step = get_absolute_time();
    
    printf("Stepper Motor Control\n");
    printf("Press button on GP18 for clockwise rotation\n");
    printf("Press button on GP19 for counter-clockwise rotation\n");
    
    while (true) {
        // Check if it's time for the next step
        if (time_reached(next_step)) {
            // Read button states (buttons are active low with pull-up)
            bool cw_pressed = !gpio_get(BUTTON_CW);
            bool ccw_pressed = !gpio_get(BUTTON_CCW);
            
            if (cw_pressed && !ccw_pressed) {
                // Clockwise rotation
                idx = (idx + 1) & 7;
                drive_mask(HALFSTEP[idx]);
                next_step = make_timeout_time_us(STEP_US);
            } else if (ccw_pressed && !cw_pressed) {
                // Counter-clockwise rotation
                idx = (idx + 7) & 7;  // +7 is same as -1 in modulo 8
                drive_mask(HALFSTEP[idx]);
                next_step = make_timeout_time_us(STEP_US);
            } else {
                // No button pressed or both pressed - turn off coils to save power
                coils_off();
                next_step = make_timeout_time_us(STEP_US);
            }
        }
        
        // Small delay to prevent busy-waiting
        sleep_us(100);
    }
}
