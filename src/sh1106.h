#ifndef SH1106_H
#define SH1106_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Display dimensions
#define SH1106_WIDTH 128
#define SH1106_HEIGHT 64
#define SH1106_PAGES 8  // 64 pixels / 8 pixels per page

// Column offset from GDDRAM to visible area varies by module; 2 is common for 1.3" SH1106
#ifndef SH1106_COL_OFFSET
#define SH1106_COL_OFFSET 2
#endif

// SH1106 Commands
// NOTE: Despite being labeled as SH1106, some modules require SSD1306-style
// charge pump commands (0x8D/0x14) for proper power-on initialization.
// This driver uses SSD1306 charge pump commands for maximum compatibility.
#define SH1106_CMD_SET_COLUMN_ADDR_LOW   0x00
#define SH1106_CMD_SET_COLUMN_ADDR_HIGH  0x10
#define SH1106_CMD_SET_PUMP_VOLTAGE      0x30
#define SH1106_CMD_SET_START_LINE        0x40
#define SH1106_CMD_SET_CONTRAST          0x81
#define SH1106_CMD_SET_SEGMENT_REMAP     0xA0  // A0/A1
#define SH1106_CMD_RESUME_FROM_RAM       0xA4  // Resume to RAM (not forced ON)
#define SH1106_CMD_ENTIRE_DISPLAY_ON     0xA5  // Force all pixels ON
#define SH1106_CMD_SET_NORMAL_DISPLAY    0xA6
#define SH1106_CMD_SET_INVERT_DISPLAY    0xA7
#define SH1106_CMD_SET_MULTIPLEX         0xA8
#define SH1106_CMD_SET_DCDC              0xAD  // SH1106 DC-DC command (not used)
#define SH1106_CMD_DISPLAY_OFF           0xAE
#define SH1106_CMD_DISPLAY_ON            0xAF
#define SH1106_CMD_SET_PAGE_ADDR         0xB0
#define SH1106_CMD_SET_COM_SCAN_DIR      0xC0  // C0/C8
#define SH1106_CMD_SET_DISPLAY_OFFSET    0xD3
#define SH1106_CMD_SET_DISPLAY_CLOCK     0xD5
#define SH1106_CMD_SET_PRECHARGE         0xD9
#define SH1106_CMD_SET_COM_PINS          0xDA
#define SH1106_CMD_SET_VCOM_DESELECT     0xDB
#define SH1106_CMD_NOP                   0xE3

// SSD1306 charge pump commands (used for compatibility)
#define SSD1306_CMD_CHARGE_PUMP         0x8D
#define SSD1306_CHARGE_PUMP_ENABLE      0x14
#define SSD1306_CHARGE_PUMP_DISABLE     0x10

// Control bytes
#define SH1106_CTRL_CMD_SINGLE  0x80
#define SH1106_CTRL_CMD_STREAM  0x00
#define SH1106_CTRL_DATA_STREAM 0x40

// SH1106 structure
typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
    uint8_t buffer[SH1106_WIDTH * SH1106_PAGES];  // Display buffer
} sh1106_t;

// Function prototypes
bool sh1106_init(sh1106_t *display, i2c_inst_t *i2c, uint8_t addr, uint8_t sda_pin, uint8_t scl_pin);
void sh1106_command(sh1106_t *display, uint8_t cmd);
void sh1106_display_on(sh1106_t *display, bool on);
void sh1106_set_contrast(sh1106_t *display, uint8_t contrast);
void sh1106_entire_display(sh1106_t *display, bool on);
void sh1106_invert(sh1106_t *display, bool invert);
void sh1106_clear(sh1106_t *display);
void sh1106_update(sh1106_t *display);
void sh1106_set_pixel(sh1106_t *display, uint8_t x, uint8_t y, bool on);
void sh1106_draw_line(sh1106_t *display, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void sh1106_draw_rect(sh1106_t *display, uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool fill);
void sh1106_draw_char(sh1106_t *display, uint8_t x, uint8_t y, char c);
void sh1106_draw_string(sh1106_t *display, uint8_t x, uint8_t y, const char *str);

#endif // SH1106_H
