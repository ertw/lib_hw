
#include "pico/stdlib.h"

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
    const uint8_t pins[4] = {PIN_IN1, PIN_IN2, PIN_IN3, PIN_IN4};
    for (int i = 0; i < 4; ++i) { gpio_init(pins[i]); gpio_set_dir(pins[i], GPIO_OUT); gpio_put(pins[i], 0); }
    int idx = 0;
    { absolute_time_t until = make_timeout_time_ms(3000);
      while (!time_reached(until)) { idx = (idx + 1) & 7; drive_mask(HALFSTEP[idx]); sleep_us(STEP_US); } }
    { absolute_time_t until = make_timeout_time_ms(3000);
      while (!time_reached(until)) { idx = (idx + 7) & 7; drive_mask(HALFSTEP[idx]); sleep_us(STEP_US); } }
    coils_off();
    while (true) { sleep_ms(1000); }
}
