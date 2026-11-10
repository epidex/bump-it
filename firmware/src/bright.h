/* SPDX-License-Identifier: GPL-3.0-only */
/* (Bump It 1.1) SETTINGS > SCREEN > BRIGHTNESS: how bright the screen looks.
 *
 * The FM-1's backlight (PA2, active low) is only on or off (no PWM wired to it), so the light is not dimmed: the
 * picture is. lcd.c scales every pixel's red, green and blue on its way to the panel (lcd_blit, lcd_fill), so every
 * screen (pages, menu, scope, splash, crash screen) follows the setting, and the canvas itself is never changed.
 * At 100% nothing is scaled: the pixels go out exactly as before, by the same single DMA.
 *
 * The levels are named by roughly how much light the panel gives (RGB565 is gamma-encoded, ~2.2): a value scaled by
 * q/256 gives about (q/256)^2.2 of the light. 100 70 50 30 15 % -> q 256 218 187 148 108.
 *
 * Saved in the settings record, in the ZOOM word (persist_t.zoom): bit 0 ZOOM, bits 8..10 the BRIGHTNESS step
 * (0 = 100%, so every older record reads 100%). */
#ifndef BUMPIT_BRIGHT_H
#define BUMPIT_BRIGHT_H
#include <stdint.h>

#define BRIGHT_N 5u                                   /* steps: 0 = 100% .. 4 = 15% */
#define BRIGHT_SHIFT 8u                               /* in settings.zoom / persist_t.zoom */
#define BRIGHT_MASK (7u << BRIGHT_SHIFT)
#define ZOOM_BIT 1u
#define ZOOM_WORD_OK(z) (((z) & ~(ZOOM_BIT | BRIGHT_MASK)) == 0u && (((z) & BRIGHT_MASK) >> BRIGHT_SHIFT) < BRIGHT_N)

static const uint16_t BRIGHT_Q8[BRIGHT_N] = {256u, 218u, 187u, 148u, 108u};
static const char *const BRIGHT_NAME[BRIGHT_N] = {"100%", "70%", "50%", "30%", "15%"};

static uint8_t lcd_bright_step;                       /* 0 = full: lcd.c sends the pixels untouched */
static uint8_t lcd_l5[32], lcd_l6[64];                /* a 5-bit / 6-bit channel value at this step */

static void lcd_bright_set(uint32_t step)             /* the tables for step (before the next blit) */
{
    uint32_t q, v;
    if (step >= BRIGHT_N)
        step = 0;
    lcd_bright_step = (uint8_t)step;
    q = BRIGHT_Q8[step];
    for (v = 0; v < 32u; v++)
        lcd_l5[v] = (uint8_t)((v * q + 128u) >> 8);
    for (v = 0; v < 64u; v++)
        lcd_l6[v] = (uint8_t)((v * q + 128u) >> 8);
}

static inline uint16_t lcd_dim565(uint32_t c)         /* an RGB565 colour (CPU order) at this step */
{
    return (uint16_t)((uint32_t)lcd_l5[(c >> 11) & 31u] << 11 | (uint32_t)lcd_l6[(c >> 5) & 63u] << 5 |
                      lcd_l5[c & 31u]);
}
#endif
