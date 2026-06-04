# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

STC89C51 (8051-compatible) LED blink + button control firmware, written in C51 for Keil uVision. A single-source embedded MCU project with no CLI build system — all compilation and flashing happens through the Keil IDE and STC-ISP tool.

## Build & Flash

- **Open project**: `led_blink.uvproj` in Keil uVision (C51 edition)
- **Build**: Press `F7` or click the Build toolbar button
- **Output**: `led_blink.hex` (HEX-8051 format) in the project root
- **Flash**: Use STC-ISP utility to download `led_blink.hex` to the MCU via UART

There is no Makefile or CLI compiler invocation — Keil C51 is the only supported toolchain.

## Hardware Pin Map

| Pin   | Signal | Direction | Notes                         |
|-------|--------|-----------|-------------------------------|
| P1.0  | LED    | Output    | Active-low (0 = on, 1 = off)  |
| P3.2  | KEY    | Input     | Active-low, internal weak pull-up |

- **Crystal**: 11.0592 MHz, 12T mode (defined as `FOSC 11059200UL`)
- **LED circuit**: Anode → VCC, cathode → 470Ω resistor → P1.0
- **KEY circuit**: One side → P3.2, other side → GND (no external pull-up needed)

## Code Structure (`main.c`)

Single-file architecture with four functional blocks:

1. **Hardware defines** (lines 15–20): `sbit` pin aliases (`LED`, `KEY`) and `FOSC` crystal frequency macro
2. **Mode state machine** (lines 23–29): Three modes (`MODE_BLINK=0`, `MODE_ON=1`, `MODE_OFF=2`) cycled by button press, tracked in global `led_mode`
3. **Utilities** (lines 37–101):
   - `delay_ms()` — software delay loop (blocking, ~1ms resolution, calibrated for 11.0592MHz/12T)
   - `key_scan()` — debounced button read with release detection (20ms debounce)
   - `key_handle()` — mode cycler (increments and wraps `led_mode`)
   - `timer0_init()` — optional hardware timer setup for precise 50ms interrupts (commented out by default)
4. **Main loop** (lines 106–143): Polls `key_scan()`, then runs a `switch(led_mode)` to drive the LED

The optional timer0 ISV (commented out at end of file) provides a non-blocking alternative to `delay_ms()` by toggling LED in a 50ms × 10 = 500ms interrupt-driven cycle.

## Key Conventions

- **8051 register access**: Uses `<reg51.h>` and `sbit` for SFR bit addressing — standard Keil C51 dialect, not portable to GCC/Clang
- **Blocking delays**: The default implementation uses busy-wait `delay_ms()`, which blocks button scanning during the 500ms half-cycles. The commented timer0 ISR is the non-blocking alternative.
- **Debounce strategy**: Release-detection (waits for button release before returning), not edge-triggered. A single press-and-hold produces exactly one mode change on release.
