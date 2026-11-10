/* SPDX-License-Identifier: GPL-3.0-only */
/* (Bump It 1.1) SETTINGS > SCREEN > BRIGHTNESS: the real lcd.c against a model of the ST7789 and its SPI DMA.
 * The model reads a transfer's bytes only when it is waited for (the latest a DMA could read them), so a buffer the
 * code rewrites while its transfer still runs shows up as wrong pixels. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static uint16_t fb[240 * 240];                 /* the panel's RAM, RGB565 */
static uint32_t wx0, wx1, wy0, wy1, wx, wy;    /* the window, the write position */
static uint8_t last_cmd, ram_on, cs_low;
static const uint8_t *pend_p;                  /* the transfer running */
static uint32_t pend_n, pix_xfers, overlaps;
static uint8_t arg[8];
static uint32_t argn;
static int fails;

static void put_byte_pair(uint8_t hi, uint8_t lo)
{
    if (wy <= wy1 && wx <= wx1 && wx < 240u && wy < 240u)
        fb[wy * 240u + wx] = (uint16_t)(hi << 8 | lo);
    if (++wx > wx1) {
        wx = wx0;
        wy++;
    }
}
static void fm1_lcd_hw_init(void) {}
static void fm1_lcd_baud(uint32_t b) { (void)b; }
static void fm1_lcd_deselect(void) { cs_low = 0; }
static void fm1_lcd_send_cmd(uint8_t c)
{
    if (pend_p)
        overlaps++;                            /* a command over a running transfer */
    last_cmd = c;
    argn = 0;
    ram_on = c == 0x2Cu;
    if (ram_on)
        wx = wx0, wy = wy0;
    cs_low = 1;
}
static void fm1_lcd_send_data(const void *p, uint32_t n)
{
    if (pend_p)
        overlaps++;
    pend_p = (const uint8_t *)p;
    pend_n = n;
    if (ram_on)
        pix_xfers++;
    cs_low = 1;
}
static void fm1_lcd_wait(void)                  /* the DMA reads its bytes now */
{
    uint32_t i;
    if (!pend_p)
        return;
    if (ram_on) {
        for (i = 0; i + 1u < pend_n; i += 2u)
            put_byte_pair(pend_p[i], pend_p[i + 1u]);
    } else {
        for (i = 0; i < pend_n && argn < 8u; i++)
            arg[argn++] = pend_p[i];
        if (last_cmd == 0x2Au && argn == 4u)
            wx0 = (uint32_t)arg[0] << 8 | arg[1], wx1 = (uint32_t)arg[2] << 8 | arg[3];
        if (last_cmd == 0x2Bu && argn == 4u)
            wy0 = (uint32_t)arg[0] << 8 | arg[1], wy1 = (uint32_t)arg[2] << 8 | arg[3];
    }
    pend_p = 0;
    pend_n = 0;
}

#include "../firmware/src/lcd.c"

static void check(int ok, const char *what)
{
    printf("%-78s %s\n", what, ok ? "ok" : "FAIL");
    if (!ok)
        fails++;
}
static uint16_t sw16(uint16_t v) { return (uint16_t)(v >> 8 | v << 8); }
static uint16_t ref_dim(uint16_t c, uint32_t q)  /* the level, worked out apart from the tables */
{
    uint32_t r = c >> 11, g = c >> 5 & 63u, b = c & 31u;
    r = (r * q + 128u) >> 8, g = (g * q + 128u) >> 8, b = (b * q + 128u) >> 8;
    return (uint16_t)(r << 11 | g << 5 | b);
}
static uint16_t img[240 * 240], img_copy[240 * 240];   /* the canvas: pixels byte-swapped, as gfx.c keeps them */

int main(void)
{
    uint32_t i, x, y, s, ok;
    srand(7);
    for (i = 0; i < 240u * 240u; i++)
        img[i] = (uint16_t)(i % 9u == 0u ? 0u : (uint32_t)rand() & 0xFFFFu);
    memcpy(img_copy, img, sizeof img);

    lcd_bright_set(0);
    for (s = 0, ok = 1; s < 32u; s++)
        ok &= lcd_l5[s] == s;
    for (s = 0; s < 64u; s++)
        ok &= lcd_l6[s] == s;
    check(ok, "100%: the tables leave every value as it is");

    pix_xfers = 0;
    lcd_blit(0, 0, 240, 240, img);
    lcd_sync();
    for (i = 0, ok = 1; i < 240u * 240u; i++)
        ok &= fb[i] == sw16(img[i]);
    check(ok, "100%: a full-screen blit reaches the panel bit for bit");
    check(pix_xfers == 1u, "100%: ... in one DMA, as before (no extra work at the default)");

    for (s = 1; s < BRIGHT_N; s++) {
        char t[120];
        uint32_t bx = 13u + s, by = 7u + s, bw = 101u + s * 17u, bh = 77u + s;
        memset(fb, 0, sizeof fb);
        lcd_bright_set(s);
        pix_xfers = 0;
        lcd_blit(bx, by, bw, bh, img);                 /* not a whole number of strips */
        lcd_blit(0, 200, 240, 40, img + 5000);          /* straight after: the strips are reused */
        lcd_sync();
        for (y = 0, ok = 1; y < bh; y++)
            for (x = 0; x < bw; x++)
                ok &= fb[(by + y) * 240u + bx + x] == ref_dim(sw16(img[y * bw + x]), BRIGHT_Q8[s]);
        for (i = 0; i < 240u * 40u; i++)
            ok &= fb[200u * 240u + i] == ref_dim(sw16(img[5000u + i]), BRIGHT_Q8[s]);
        snprintf(t, sizeof t, "%s: two blits back to back: every pixel scaled, none torn (%u + %u strips)", BRIGHT_NAME[s],
                 (bw * bh + 239u) / 240u, 40u);
        check(ok, t);
        check(pix_xfers == (bw * bh + 239u) / 240u + 40u, "   ... strips of 240 pixels, as planned");
    }
    check(overlaps == 0u, "no command or transfer started over a running transfer");
    check(!memcmp(img, img_copy, sizeof img), "the canvas itself is never changed (no dimming twice)");

    lcd_bright_set(3);
    lcd_fill(10, 10, 50, 20, 0xFFFFu);
    lcd_fill(100, 100, 5, 5, 0x0000u);
    lcd_sync();
    check(fb[15u * 240u + 20u] == ref_dim(0xFFFFu, BRIGHT_Q8[3]) && fb[102u * 240u + 102u] == 0u,
          "lcd_fill: white at 30% is the scaled white, black stays black");

    for (s = 1, ok = 1; s < BRIGHT_N; s++) {
        double l = pow(BRIGHT_Q8[s] / 256.0, 2.2) * 100.0, want = atof(BRIGHT_NAME[s]);
        ok &= fabs(l - want) < 3.0 && BRIGHT_Q8[s] < BRIGHT_Q8[s - 1u];
    }
    check(ok, "the steps get dimmer, each within 3 points of its name's light (gamma 2.2)");
    {
        uint32_t z;
        for (z = 0, ok = 1; z < 0x2000u; z++) {
            int want = (z & ~0x701u) == 0u && ((z >> 8) & 7u) < BRIGHT_N;
            ok &= ZOOM_WORD_OK(z) == want;
        }
        check(ok && ZOOM_WORD_OK(0u) && ZOOM_WORD_OK(1u) && ZOOM_WORD_OK(0x401u) && !ZOOM_WORD_OK(0x501u) && !ZOOM_WORD_OK(2u),
              "the settings word: ZOOM bit 0, BRIGHTNESS 0..4 in bits 8..10, nothing else");
    }
    printf("%s\n", fails ? "BRIGHTNESS: FAILED" : "BRIGHTNESS: all passed");
    return fails != 0;
}
