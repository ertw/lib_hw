/*
 * Minimal Working OLED Demo for SH1106 1.3" Display
 * 
 * Verified Configuration:
 * - Controller: SH1106 at I2C address 0x3C
 * - Power: 5V (VBUS pin 40)
 * - I2C: i2c1 on GP6 (SDA) and GP7 (SCL)
 * - Speed: 100 kHz
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7
#define OLED_ADDR 0x3C

// Initialize and display stripes pattern
int main() {
    // Initialize USB serial (optional for debugging)
    stdio_init_all();
    sleep_ms(2000);
    
    // Initialize I2C1 at 100kHz
    i2c_init(i2c1, 100000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    sleep_ms(100);
    
    uint8_t data[3];
    
    // === INITIALIZATION SEQUENCE (PROVEN WORKING) ===
    
    // Display OFF
    data[0] = 0x00; data[1] = 0xAE;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    sleep_ms(10);
    
    // Set display clock divide ratio
    data[0] = 0x00; data[1] = 0xD5; data[2] = 0x80;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Set multiplex ratio (64 lines)
    data[0] = 0x00; data[1] = 0xA8; data[2] = 0x3F;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Set display offset
    data[0] = 0x00; data[1] = 0xD3; data[2] = 0x00;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Set start line
    data[0] = 0x00; data[1] = 0x40;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    
    // Enable charge pump (CRITICAL!)
    data[0] = 0x00; data[1] = 0xAD;  // DC-DC command
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    data[0] = 0x00; data[1] = 0x8B;  // DC-DC ON
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    sleep_ms(100);  // Wait for charge pump
    
    // Set segment remap
    data[0] = 0x00; data[1] = 0xA1;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    
    // Set COM scan direction
    data[0] = 0x00; data[1] = 0xC8;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    
    // Set COM pins configuration
    data[0] = 0x00; data[1] = 0xDA; data[2] = 0x12;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Set contrast (maximum)
    data[0] = 0x00; data[1] = 0x81; data[2] = 0xFF;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Set precharge period
    data[0] = 0x00; data[1] = 0xD9; data[2] = 0xF1;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Set VCOMH deselect level
    data[0] = 0x00; data[1] = 0xDB; data[2] = 0x40;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 3, false);
    
    // Display follows RAM
    data[0] = 0x00; data[1] = 0xA4;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    
    // Normal display (not inverted)
    data[0] = 0x00; data[1] = 0xA6;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    
    // === DRAW STRIPE PATTERN ===
    uint8_t buf[129];
    buf[0] = 0x40;  // Data mode control byte
    
    for (uint8_t page = 0; page < 8; page++) {
        // Set page address
        data[0] = 0x00; data[1] = 0xB0 | page;
        i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
        
        // Set column address (with offset 2 for SH1106)
        data[0] = 0x00; data[1] = 0x02;  // Low nibble
        i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
        data[0] = 0x00; data[1] = 0x10;  // High nibble
        i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
        
        // Create stripe pattern
        for (int i = 1; i <= 128; i++) {
            buf[i] = (i % 8 < 4) ? 0xFF : 0x00;  // Vertical stripes
        }
        
        // Write data to display
        i2c_write_blocking(i2c1, OLED_ADDR, buf, 129, false);
    }
    
    // Display ON
    data[0] = 0x00; data[1] = 0xAF;
    i2c_write_blocking(i2c1, OLED_ADDR, data, 2, false);
    
    // Keep running
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
}
