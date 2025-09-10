# SH1106 OLED Controller — Quick Reference (v2.3 summary)

> Source: Sino Wealth SH1106 datasheet v2.3 and field notes.  
> PDF originals for full detail: Velleman/Pololu mirrors. Values below are the usual ones for 128×64 modules. 

## Overview
- 132×64 dot-matrix OLED/PLED segment/common driver with controller. Designed for **common-cathode** panels.  
- Internal display RAM: 132×64 bits (8 “pages” × 132 columns). Most **128×64 modules show 128 columns**, leaving 4 columns off-panel; many need a small X offset when drawing. :contentReference[oaicite:1]{index=1}

## Supplies & ranges (typical, per datasheet)
- Logic: **VDD1 = 1.65–3.5 V**  
- Charge-pump supply: **VDD2 = 3.0–4.2 V**  
- OLED VPP (panel): **6.4–9.0 V** from internal pump or **6.4–14.0 V** external.  
- Max segment current ~200 µA (256 steps via contrast); wide temp −40 °C to +85 °C. :contentReference[oaicite:2]{index=2}

## Interfaces
- 8-bit **8080** / **6800** parallel, **SPI (3-wire/4-wire)**, **I²C**.  
- Interface mode via **IM2..IM0** pins.  
- I²C addresses commonly **0x3C** or **0x3D** depending on SA0/“DC” wiring. :contentReference[oaicite:3]{index=3}

## Memory addressing
- **Pages:** 0–7 (each page = 8 pixel rows).  
- **Columns:** 0–131 (only 0–127 are visible on typical 128-wide modules).  
- Column is set by *Lower Column* (0x00–0x0F) and *Higher Column* (0x10–0x1F); page by *Set Page Address* (0xB0–0xB7). :contentReference[oaicite:4]{index=4}

## Power/Init notes (common sequence)
1. **Display OFF** (0xAE).  
2. Set **clock divide/osc** (0xD5, data).  
3. Set **multiplex ratio** (0xA8, 1–64; 0x3F for 64).  
4. Set **display offset** (0xD3, usually 0).  
5. Set **start line** (0x40 | n, usually 0).  
6. Set **segment remap** (0xA0/0xA1) and **COM scan dir** (0xC0/0xC8) to match your module.  
7. Configure **DC-DC**: control (0xAD), ON/OFF mode (0x8A/0x8B) as required; then contrast (0x81, data).  
8. **Normal display** (0xA6) and **Entire Display ON** (0xA4).  
9. **Display ON** (0xAF). :contentReference[oaicite:5]{index=5}

> Tip (128×64 panels on SH1106): because RAM is 132 wide, many breakout boards wire the visible area starting at column 2. Add **+2 to X** or set your column to 2 before drawing each row. :contentReference[oaicite:6]{index=6}

## Command set (hex)
- **Column Addressing**
  - Set **Lower Column**: `0x00–0x0F`  
  - Set **Higher Column**: `0x10–0x1F`  — combine to form 0–131. :contentReference[oaicite:7]{index=7}
- **Page Addressing**
  - Set **Page Address**: `0xB0–0xB7` (page 0–7). :contentReference[oaicite:8]{index=8}
- **Display Start Line**
  - Set **Start Line**: `0x40–0x7F` (line 0–63). :contentReference[oaicite:9]{index=9}
- **Contrast**
  - **Enable contrast mode**: `0x81`  
  - **Contrast data**: `0x00–0xFF` (256 steps). :contentReference[oaicite:10]{index=10}
- **Segment remap (X flip)**
  - `0xA0` (ADC=0, normal) / `0xA1` (ADC=1, mirror). :contentReference[oaicite:11]{index=11}
- **Entire display ON**
  - `0xA4` (follow RAM) / `0xA5` (force all pixels ON). :contentReference[oaicite:12]{index=12}
- **Normal/Inverse**
  - `0xA6` (normal) / `0xA7` (inverse). :contentReference[oaicite:13]{index=13}
- **Multiplex ratio (display height)**
  - **Mode set**: `0xA8`  
  - **Data**: `0x00–0x3F` → 1–64 lines (POR=64). :contentReference[oaicite:14]{index=14}
- **DC-DC (charge pump)**
  - **Control mode set**: `0xAD`  
  - **ON/OFF mode**: `0x8A`/`0x8B` (see table; typically ON when display ON). :contentReference[oaicite:15]{index=15}
- **Display ON/OFF (sleep)**
  - `0xAE` (OFF → enter power-save) / `0xAF` (ON). :contentReference[oaicite:16]{index=16}
- **COM scan direction (Y flip)**
  - `0xC0` (COM0→COM[N–1], normal) / `0xC8` (reverse). :contentReference[oaicite:17]{index=17}
- **Display offset (vertical scroll baseline)**
  - **Mode set**: `0xD3`; **Data**: `0x00–0x3F`. :contentReference[oaicite:18]{index=18}
- **Display clock divide / oscillator frequency**
  - **Mode set**: `0xD5`; **Data**: `A7..A4=fosc tweak, A3..A0=divide (1–16)`. POR divide=1. :contentReference[oaicite:19]{index=19}
- **Pre-charge & Dis-charge periods**
  - **Mode set**: `0xD9`; **Data**: `A7..A4=discharge, A3..A0=precharge` (in DCLKs; POR precharge=2). :contentReference[oaicite:20]{index=20}
- **Common pads hardware config (sequential/alternative)**
  - **Mode set**: `0xDA`; **Data**: `0x02–0x12` (D=0 sequential, D=1 alternative; POR alternative). :contentReference[oaicite:21]{index=21}
- **VCOM deselect level**
  - **Mode set**: `0xDB`; **Data**: selects VCOMH via β (POR ≈ 0.77×Vref). :contentReference[oaicite:22]{index=22}

## Typical I²C addresses
- Many modules: **0x3C** (DC/SA0=0) or **0x3D** (DC/SA0=1). :contentReference[oaicite:23]{index=23}

## Notes
- After reset, defaults include: display OFF; column=0; start line=0; normal scan; contrast ~0x80; internal DC-DC selected. :contentReference[oaicite:24]{index=24}
- For SPI 3-wire, data/command is multiplexed on SDIN; for I²C, the A0/“DC” pin becomes SA0 (address LSB). :contentReference[oaicite:25]{index=25}

