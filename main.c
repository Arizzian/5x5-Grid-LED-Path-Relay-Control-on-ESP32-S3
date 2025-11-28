#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PIN_DATA   GPIO_NUM_4
#define PIN_CLOCK  GPIO_NUM_5
#define PIN_LATCH  GPIO_NUM_6

#define NUM_COLS 5
#define NUM_ROWS 5
#define NUM_PIXELS (NUM_COLS * NUM_ROWS)
#define NUM_CHIPS 4   // 4 * 8 = 32 outputs, enough for 25 LEDs

static uint8_t frame[NUM_CHIPS];

// Initialize GPIO pins for the shift registers
static void io_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<PIN_DATA) | (1ULL<<PIN_CLOCK) | (1ULL<<PIN_LATCH),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    gpio_set_level(PIN_DATA, 0);
    gpio_set_level(PIN_CLOCK, 0);
    gpio_set_level(PIN_LATCH, 0);
}

// Shift out one byte MSB-first
// Note: using vTaskDelay(1ms) for stable timing in Wokwi/editor (avoids esp_rom headers)
static void shift_out_byte(uint8_t b)
{
    for (int i = 7; i >= 0; --i) {
        gpio_set_level(PIN_CLOCK, 0);
        int bit = (b >> i) & 1;
        gpio_set_level(PIN_DATA, bit);
        vTaskDelay(pdMS_TO_TICKS(1));   // 1 ms -> stable for simulator
        gpio_set_level(PIN_CLOCK, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Latch the shifted data into the outputs
static void latch(void)
{
    gpio_set_level(PIN_LATCH, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(PIN_LATCH, 1);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(PIN_LATCH, 0);
}

// Send the frame to all shift registers (chip N-1 first)
static void send_frame(void)
{
    for (int chip = NUM_CHIPS - 1; chip >= 0; --chip) {
        shift_out_byte(frame[chip]);
    }
    latch();
}

// Set a single LED pixel in the 5x5 grid (x=col, y=row)
static void set_pixel(int x, int y, int value)
{
    if (x < 0 || x >= NUM_COLS || y < 0 || y >= NUM_ROWS) return;

    int index = x * NUM_ROWS + y;  // column-major mapping
    int chip = index / 8;          // which 74HC595 (0..NUM_CHIPS-1)
    int bit  = index % 8;          // which bit inside that chip

    if (chip < 0 || chip >= NUM_CHIPS) return;

    if (value)
        frame[chip] |= (1 << bit);
    else
        frame[chip] &= ~(1 << bit);
}

// Clear all LEDs
static void clear_frame(void)
{
    for (int i = 0; i < NUM_CHIPS; ++i) frame[i] = 0;
}

void app_main(void)
{
    io_init();
    clear_frame();

    // Simple diagonal bouncing demo (5x5)
    int x = 0, y = 0, dx = 1, dy = 1;

    printf("Starting 5x5 LED matrix demo\n");

    while (1) {
        clear_frame();
        set_pixel(x, y, 1);
        send_frame();

        // simple debug output (matches the template style)
        printf("Puck at (%d,%d)\n", x, y);

        // move the puck
        x += dx;
        y += dy;

        if (x < 0) { x = 0; dx = 1; }
        if (x >= NUM_COLS) { x = NUM_COLS - 1; dx = -1; }
        if (y < 0) { y = 0; dy = 1; }
        if (y >= NUM_ROWS) { y = NUM_ROWS - 1; dy = -1; }

        vTaskDelay(pdMS_TO_TICKS(160)); // main frame rate
    }
}
