#include "../lib.h"
#include <string.h>
#include <stdlib.h>

// Basic 5x7 font (ASCII 32-127)
static const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x10, 0x08, 0x08, 0x10, 0x08}, // ~
    {0x78, 0x46, 0x41, 0x46, 0x78}  // DEL
};

// Send command to display
hw_result_t sh1106_command(sh1106_t *display, uint8_t cmd) {
    // Use 0x00 control byte - confirmed working with your display
    uint8_t data[2] = {0x00, cmd};
    int ret = i2c_write_timeout_us(display->i2c, display->addr, data, 2, false, SH1106_I2C_TIMEOUT_US);
    return (ret == 2) ? HW_OK : HW_ERROR;
}

// Initialize the display
hw_result_t sh1106_init(sh1106_t *display, i2c_inst_t *i2c, uint8_t addr, uint8_t sda_pin, uint8_t scl_pin) {
    // Store configuration
    display->i2c = i2c;
    display->addr = addr;
    
    // Initialize I2C
    i2c_init(i2c, SH1106_I2C_FREQ);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    
    // Give display time to power up
    sleep_ms(100);
    
    // Check if device is present using write (not read which hangs)
    int ret = i2c_write_timeout_us(i2c, addr, NULL, 0, false, SH1106_I2C_TIMEOUT_US);
    if (ret < 0) {
        return HW_NOT_FOUND;  // Device not found
    }
    
    // Initialize display with correct command sequence
    // IMPORTANT: This display requires SSD1306-style charge pump commands (0x8D/0x14)
    // even though it's labeled as SH1106. This is critical for power-on reliability.
    
    // Display off
    uint8_t cmd_off[2] = {0x00, 0xAE};
    i2c_write_blocking(display->i2c, display->addr, cmd_off, 2, false);
    sleep_ms(10);
    
    // Set display clock divide ratio/oscillator frequency
    uint8_t clock[3] = {0x00, 0xD5, 0x80};
    i2c_write_blocking(display->i2c, display->addr, clock, 3, false);
    
    // Set multiplex ratio (1 to 64)
    uint8_t mux[3] = {0x00, 0xA8, 0x3F};  // 64 lines
    i2c_write_blocking(display->i2c, display->addr, mux, 3, false);
    
    // Set display offset
    uint8_t offset[3] = {0x00, 0xD3, 0x00};
    i2c_write_blocking(display->i2c, display->addr, offset, 3, false);
    
    // Set start line address
    uint8_t startline[2] = {0x00, 0x40};
    i2c_write_blocking(display->i2c, display->addr, startline, 2, false);
    
    // CRITICAL: Enable charge pump using SSD1306 commands
    // This module requires these specific commands to work after power cycle
    uint8_t pump_cmd[2] = {0x00, 0x8D};  // Charge pump command
    i2c_write_blocking(display->i2c, display->addr, pump_cmd, 2, false);
    uint8_t pump_enable[2] = {0x00, 0x14};  // Enable charge pump
    i2c_write_blocking(display->i2c, display->addr, pump_enable, 2, false);
    sleep_ms(100);  // Wait for charge pump to stabilize
    
    // Set segment remap (column address 127 mapped to SEG0)
    uint8_t remap[2] = {0x00, 0xA1};
    i2c_write_blocking(display->i2c, display->addr, remap, 2, false);
    
    // Set COM output scan direction (remapped mode)
    uint8_t comscan[2] = {0x00, 0xC8};
    i2c_write_blocking(display->i2c, display->addr, comscan, 2, false);
    
    // Set COM pins hardware configuration
    uint8_t compins[3] = {0x00, 0xDA, 0x12};
    i2c_write_blocking(display->i2c, display->addr, compins, 3, false);
    
    // Set contrast control
    uint8_t contrast[3] = {0x00, 0x81, 0xFF};  // Maximum contrast
    i2c_write_blocking(display->i2c, display->addr, contrast, 3, false);
    
    // Set pre-charge period
    uint8_t precharge[3] = {0x00, 0xD9, 0xF1};
    i2c_write_blocking(display->i2c, display->addr, precharge, 3, false);
    
    // Set VCOMH deselect level
    uint8_t vcomh[3] = {0x00, 0xDB, 0x40};
    i2c_write_blocking(display->i2c, display->addr, vcomh, 3, false);
    
    // Display RAM content (resume from RAM)
    uint8_t resume[2] = {0x00, 0xA4};
    i2c_write_blocking(display->i2c, display->addr, resume, 2, false);
    
    // Normal display mode (not inverted)
    uint8_t normal[2] = {0x00, 0xA6};
    i2c_write_blocking(display->i2c, display->addr, normal, 2, false);
    
    // Clear the buffer
    sh1106_clear(display);
    sh1106_update(display);
    
    // Turn on display
    uint8_t cmd_on[2] = {0x00, 0xAF};
    i2c_write_blocking(display->i2c, display->addr, cmd_on, 2, false);
    
    return HW_OK;
}

// Turn display on or off
hw_result_t sh1106_display_on(sh1106_t *display, bool on) {
    return sh1106_command(display, on ? SH1106_CMD_DISPLAY_ON : SH1106_CMD_DISPLAY_OFF);
}

// Entire display on/off (test pattern)
hw_result_t sh1106_entire_display(sh1106_t *display, bool on) {
    return sh1106_command(display, on ? SH1106_CMD_ENTIRE_DISPLAY_ON : SH1106_CMD_RESUME_FROM_RAM);
}

// Invert display
hw_result_t sh1106_invert(sh1106_t *display, bool invert) {
    return sh1106_command(display, invert ? SH1106_CMD_SET_INVERT_DISPLAY : SH1106_CMD_SET_NORMAL_DISPLAY);
}

// Set display contrast
hw_result_t sh1106_set_contrast(sh1106_t *display, uint8_t contrast) {
    hw_result_t ret = sh1106_command(display, SH1106_CMD_SET_CONTRAST);
    if (ret != HW_OK) return ret;
    return sh1106_command(display, contrast);
}

// Clear the display buffer
void sh1106_clear(sh1106_t *display) {
    memset(display->buffer, 0, sizeof(display->buffer));
}

// Update the display with buffer contents (chunked writes for compatibility)
hw_result_t sh1106_update(sh1106_t *display) {
    for (uint8_t page = 0; page < SH1106_PAGES; page++) {
        // Set page address
        if (sh1106_command(display, SH1106_CMD_SET_PAGE_ADDR | page) != HW_OK) {
            return HW_ERROR;
        }

        // Set column start based on offset (many SH1106 modules use 2)
        if (sh1106_command(display, SH1106_CMD_SET_COLUMN_ADDR_HIGH | ((SH1106_COL_OFFSET >> 4) & 0x0F)) != HW_OK) {
            return HW_ERROR;
        }
        if (sh1106_command(display, SH1106_CMD_SET_COLUMN_ADDR_LOW | (SH1106_COL_OFFSET & 0x0F)) != HW_OK) {
            return HW_ERROR;
        }

        // Write page data in small chunks (16 bytes)
        const uint8_t chunk = 16;
        for (uint8_t x = 0; x < SH1106_WIDTH; x += chunk) {
            uint8_t len = (x + chunk <= SH1106_WIDTH) ? chunk : (SH1106_WIDTH - x);
            uint8_t data[1 + 16];
            data[0] = SH1106_CTRL_DATA_STREAM;  // Data control byte
            memcpy(&data[1], &display->buffer[page * SH1106_WIDTH + x], len);
            
            int ret = i2c_write_timeout_us(display->i2c, display->addr, data, 1 + len, false, SH1106_I2C_TIMEOUT_US);
            if (ret != (1 + len)) {
                return HW_ERROR;
            }
        }
    }
    return HW_OK;
}

// Set a pixel in the buffer
void sh1106_set_pixel(sh1106_t *display, uint8_t x, uint8_t y, bool on) {
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;
    
    uint16_t index = (y / 8) * SH1106_WIDTH + x;
    uint8_t bit = y % 8;
    
    if (on) {
        display->buffer[index] |= (1 << bit);
    } else {
        display->buffer[index] &= ~(1 << bit);
    }
}

// Draw a line using Bresenham's algorithm
void sh1106_draw_line(sh1106_t *display, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        sh1106_set_pixel(display, x0, y0, on);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw a rectangle
void sh1106_draw_rect(sh1106_t *display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool fill) {
    if (fill) {
        for (uint8_t i = 0; i < h; i++) {
            for (uint8_t j = 0; j < w; j++) {
                sh1106_set_pixel(display, x + j, y + i, true);
            }
        }
    } else {
        // Top and bottom lines
        for (uint8_t i = 0; i < w; i++) {
            sh1106_set_pixel(display, x + i, y, true);
            sh1106_set_pixel(display, x + i, y + h - 1, true);
        }
        // Left and right lines
        for (uint8_t i = 0; i < h; i++) {
            sh1106_set_pixel(display, x, y + i, true);
            sh1106_set_pixel(display, x + w - 1, y + i, true);
        }
    }
}

// Draw a character at specified position
void sh1106_draw_char(sh1106_t *display, uint8_t x, uint8_t y, char c) {
    // Limit to printable ASCII
    if (c < 32 || c > 127) c = 32;
    
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t column = font5x7[c - 32][col];
        for (uint8_t row = 0; row < 8; row++) {
            if (column & (1 << row)) {
                sh1106_set_pixel(display, x + col, y + row, true);
            }
        }
    }
}

// Draw a string at specified position
void sh1106_draw_string(sh1106_t *display, uint8_t x, uint8_t y, const char *str) {
    uint8_t x_pos = x;
    while (*str) {
        if (x_pos + 5 > SH1106_WIDTH) {
            x_pos = x;
            y += 8;
            if (y + 8 > SH1106_HEIGHT) break;
        }
        sh1106_draw_char(display, x_pos, y, *str);
        x_pos += 6;  // Character width + spacing
        str++;
    }
}
