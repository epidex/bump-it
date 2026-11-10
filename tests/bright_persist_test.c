/* SPDX-License-Identifier: GPL-3.0-only */
/* (Bump It 1.1) BRIGHTNESS survives a power-off: saved with the settings (the ZOOM word, bits 8..10), read back by the
 * boot (persist_boot, settings_init), taken by the web editor's settings restore, and anything else refused.
 * On the editor harness (tests/editor_host.c), its flash model. Exit status: failed checks. */
#include "editor_host.c"
static int fails;
static void check(int c, const char *what) { printf("bright_persist: %-80s %s\n", what, c ? "ok" : "FAIL"); fails += !c; }

int main(void)
{
    persist_t p;
    harness_init();
    memset(nor, 0xFF, sizeof nor);                 /* a first install */
    persist_boot();
    settings_init();
    check(bright_step() == 0u && lcd_bright_step == 0u && !zoom_on(), "first power-on: 100%, ZOOM OFF");

    settings.zoom = ZOOM_BIT | 3u << BRIGHT_SHIFT;  /* SETTINGS: ZOOM ON, BRIGHTNESS 30% */
    bright_apply();
    settings_save();
    memset(&settings, 0, sizeof settings);
    lcd_bright_set(0);
    persist_boot();                                 /* the next power-on */
    settings_init();
    check(bright_step() == 3u && lcd_bright_step == 3u && zoom_on(), "power-on after a save: 30% and ZOOM ON, as saved");

    persist_fill(&p);
    check(p.zoom == (ZOOM_BIT | 3u << BRIGHT_SHIFT), "the record holds both in its ZOOM word");
    p.zoom = 1u << BRIGHT_SHIFT;                     /* the editor sends the settings back: 70%, ZOOM OFF */
    check(settings_restore(&p, sizeof p) == 0u && bright_step() == 1u && lcd_bright_step == 1u && !zoom_on(),
          "web editor settings restore: taken, the screen at 70% at once");
    p.zoom = 5u << BRIGHT_SHIFT;
    check(settings_restore(&p, sizeof p) == 2u && bright_step() == 1u, "a level that is not one (6th step): refused");
    p.zoom = 2u;
    check(settings_restore(&p, sizeof p) == 2u, "other bits in the ZOOM word: refused");
    p.zoom = 1u;
    check(settings_restore(&p, sizeof p) == 0u && zoom_on() && bright_step() == 0u && lcd_bright_step == 0u,
          "a record from before (ZOOM 1): taken, 100%");
    printf("bright_persist: %s\n", fails ? "FAILED" : "all passed");
    return fails;
}
