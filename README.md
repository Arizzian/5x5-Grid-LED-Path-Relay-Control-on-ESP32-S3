# 5x5-Grid-LED-Path-Relay-Control-on-ESP32-S3

**Overview:**
- **What:** A small ESP32-S3 demo that drives a 5x5 LED grid using chained 74HC595 shift registers. The firmware runs a simple diagonal "puck" animation and demonstrates pixel addressing via column-major mapping.
- **Why:** Shows how to expand GPIO outputs with 74HC595 ICs and control an LED matrix from the ESP32-S3 using bit-shifting.

**Repository layout:**
- `CMakeLists.txt`: ESP-IDF component registration (build config).
- `main.c`: Application source (IO, shift register routines, demo loop).
- `diagram.json`: Wokwi wiring diagram for the ESP32 and 74HC595 chain.

**Hardware required:**
- ESP32-S3 devkit (or compatible ESP32-S3 board)
- 74HC595 shift registers (chained) — the code uses 4 chips (`NUM_CHIPS = 4`), but the diagram may show 5; 4 chips (4×8=32 outputs) are sufficient for 25 LEDs.
- 25 LEDs and current-limiting resistors (≈220 Ω)
- Power supply 3.3V, common ground
- Hookup wires and breadboard or PCB

**Wiring (summary):**
- Connect all 74HC595 `VCC` pins to `3.3V` and all `GND` pins to the ESP32 ground.
- Tie `OE` (output enable) of the 74HC595s to GND (active low).
- Tie `MR` (master reset) to `3.3V` (inactive high) — or keep it high for normal operation.
- Chain the shift registers: `Q7S` (serial out) of chip N → `DS` (serial in) of chip N+1.
- ESP32 pin assignments (as defined in `main.c`):
	- `PIN_DATA`  = GPIO4  -> 74HC595 `DS` (serial data in)
	- `PIN_CLOCK` = GPIO5  -> 74HC595 `SHCP` (shift clock)
	- `PIN_LATCH` = GPIO6  -> 74HC595 `STCP` (store/latch clock)

Note: These pins are defined in `main.c` and can be changed to other free GPIOs if needed.

**Code summary (key points in `main.c`):
- `NUM_COLS = 5`, `NUM_ROWS = 5`, `NUM_CHIPS = 4`.
- Frame buffer: `static uint8_t frame[NUM_CHIPS];` — each byte maps to one 74HC595 outputs.
- Mapping: column-major index: `index = x * NUM_ROWS + y`.
- Chip and bit: `chip = index / 8; bit = index % 8` — set/clear bits with `set_pixel(x,y,value)`.
- `shift_out_byte()` sends one byte MSB-first; `send_frame()` shifts all chips (highest chip first) then latches.
- Demo: a bouncing diagonal pixel (prints position and updates at ~160 ms per frame).

**Build & flash (ESP-IDF):**
1. Install and set up ESP-IDF per Espressif instructions: https://docs.espressif.com
2. From the project root (this repo) run:

```bash
# set target to esp32s3 (if needed)
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with your serial device. If you use the ESP-IDF devcontainer or VS Code extension, use the extension UI to build/flash.

**Tuning & notes for real hardware:**
- `shift_out_byte()` currently uses `vTaskDelay(pdMS_TO_TICKS(1))` for stable timing (suitable for simulators like Wokwi). For faster/real-hardware operation, use tighter delays (e.g., `ets_delay_us()` or direct GPIO register toggling) and test signal integrity.
- If you change `NUM_CHIPS`, update the wiring (number of 74HC595 chained) and ensure `frame[]` is sized accordingly.
- Be mindful of current limits: do not drive too many LEDs at full current simultaneously. Use multiplexing, resistors, or driver chips if necessary.

**How to modify behavior:**
- Change the animation in `app_main()` or add functions that compute `frame[]` patterns and call `send_frame()` on each loop.
- Use `set_pixel(x,y,1)` to set a pixel and `set_pixel(x,y,0)` to clear it.

**Troubleshooting:**
- No LEDs light: check power rails (3.3V), common ground, `OE` tied to GND, `MR` tied high, and correct data/clock/latch pins.
- Strange bit order: code shifts MSB-first; if your wiring or chip orientation differs, try reversing bit order or chip ordering in `send_frame()`.

**Credits & origin:**
- Author: Aryan (diagram metadata)
- Small demo adapted for ESP32-S3 using ESP-IDF.

**License:**
- This repository does not include a license file. Add one if you plan to publish or share this project publicly.

---
If you want, I can also:
- Add a wiring diagram image to the repo (rendered from `diagram.json`).
- Speed up `shift_out_byte()` for real hardware and test timings.
- Add a small CLI or simple web interface to toggle patterns.
