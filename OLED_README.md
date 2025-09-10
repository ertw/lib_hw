# SH1106 OLED Display Demo

Successfully configured and tested 1.3" OLED display with SH1106 controller.

## Hardware Configuration

### Display Specifications
- **Controller**: SH1106 (128x64 pixels)
- **Interface**: I2C
- **Address**: 0x3C
- **Size**: 1.3 inch
- **Power**: Requires 5V (has onboard voltage regulator)
- **Note**: Some SH1106 modules require SSD1306-style charge pump commands

### Wiring (Pico 2 W)
| OLED Pin | Pico Pin | Description |
|----------|----------|-------------|
| VCC | Pin 40 (VBUS) | 5V power supply |
| GND | Any GND | Ground |
| SDA | Pin 9 (GP6) | I2C1 Data |
| SCL | Pin 10 (GP7) | I2C1 Clock |

**Important**: This display requires 5V power (VBUS), not 3.3V. The module has an onboard voltage regulator.

## Software Configuration

### Key Learnings
1. **I2C Controller**: Must use `i2c1` for GP6/GP7 pins (not i2c0)
2. **Charge Pump**: Despite being labeled SH1106, this module requires SSD1306-style charge pump commands (0x8D followed by 0x14) for reliable power-on initialization. The standard SH1106 commands (0xAD/0x8B) do not survive power cycles.
3. **Control Bytes**: Uses 0x00 for commands, 0x40 for data
4. **Column Offset**: SH1106 has 132 columns, display shows 128 (offset of 2)
5. **Power Requirements**: Must use 5V from VBUS pin (pin 40), not 3.3V

### Building
```bash
make build
```

### Flashing
```bash
make flash
# Or manually copy build/oled_demo.uf2 to the Pico in BOOTSEL mode
```

## Demo Features

The `oled_demo` executable demonstrates:
- Display initialization with charge pump
- Text rendering with 5x7 font
- Graphics primitives (lines, rectangles)
- Animation (rotating box, bouncing ball)
- Multiple demo screens cycling through:
  - Welcome screen
  - System information
  - Graphics demo with animation
  - Character set display
  - Contrast testing

## Library Files

- `sh1106.h` - Driver header with command definitions
- `sh1106.c` - Driver implementation with graphics functions
- `oled_demo.c` - Demo application showcasing features

## Troubleshooting

If the display doesn't work:
1. **Power**: Verify 5V power connection from VBUS (pin 40), not 3.3V
2. **I2C Address**: Check I2C address with scanner (should be 0x3C)
3. **I2C Controller**: Ensure using i2c1 for GP6/GP7 pins (not i2c0)
4. **Charge Pump**: The driver uses SSD1306-style charge pump commands (0x8D/0x14) which are required for this module to work after power cycling
5. **Wiring**: Double-check all connections - intermittent failures often indicate loose connections

## API Reference

### Initialization
```c
sh1106_t display;
sh1106_init(&display, i2c1, 0x3C, 6, 7);  // i2c1, addr, SDA, SCL
```

### Basic Operations
```c
sh1106_clear(&display);                   // Clear buffer
sh1106_update(&display);                  // Send buffer to display
sh1106_display_on(&display, true/false);  // Display on/off
sh1106_set_contrast(&display, 0-255);     // Set contrast
sh1106_invert(&display, true/false);      // Invert display
```

### Graphics
```c
sh1106_set_pixel(&display, x, y, on);
sh1106_draw_line(&display, x0, y0, x1, y1, on);
sh1106_draw_rect(&display, x, y, w, h, fill);
sh1106_draw_char(&display, x, y, 'A');
sh1106_draw_string(&display, x, y, "Hello");
```
