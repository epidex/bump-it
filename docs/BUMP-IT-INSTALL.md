# Bump It 1.1 install guide

This is a polish of Bump It 1.1, not a new release: the version still reads BUMP IT 1.1 everywhere. It adds a SETTINGS menu with a screen brightness control and cuts the boot screen time by half a second. Everything else is as before: full SLOOP 2.8.5, the purple theme, the BUMP IT boot screen, the frequency-coloured oscilloscope, and no Bluetooth. It has been built and tested in a simulator only, never on a real FM-1.

## What's new in this polish

- **SETTINGS menu:** holding HOME opens the same menu as before, now headed SETTINGS instead of MENU. Its sections are unchanged: SCREEN, LIGHTS, AUDIO, SEQ and SYSTEM.
- **Brightness:** a new BRIGHTNESS row in SETTINGS > SCREEN, on KNOB 4. It has five levels: 100%, 70%, 50%, 30% and 15%. Five small bars next to the value show the level. The whole screen changes at once, including the pages, the menu, the scope and the boot screen.
- **It's remembered:** the brightness is saved with your other settings and comes back at power-on. The boot screen also comes up at your chosen level.
- **Faster start:** the boot screen now stays up for 0.4 seconds instead of 0.9, so you can play about half a second sooner. Skiff, another firmware built on the same base, holds its boot screen for 0.4 seconds too.

## Using brightness

1. Hold **HOME** to open SETTINGS. It opens on SCREEN.
2. Turn **KNOB 4** left to dim the screen and right to brighten it. It stops at 15% and at 100%.
3. Or select BRIGHTNESS and press **OCT+** to step through the levels. After 15% it goes back to 100%.

**How it dims:** the FM-1's backlight can only be on or off, so the light itself can't be turned down. Bump It darkens every colour on its way to the screen instead. Black stays black, so the panel's glow behind black areas doesn't change. The percentages are about how much light the colours give off. At 100%, the screen works exactly as it did before.

## What else was looked at

- **SLOOP:** Bump It is already built on the newest SLOOP. Its source is SLOOP 2.8.5, which is still the latest version on SLOOP's GitHub, so all of SLOOP's own polish is already in it.
- **Skiff:** [Skiff](https://github.com/illuminus5/skiff) is a six-track groovebox firmware for the FM-1, also GPL-3.0. Its shorter boot screen is the one idea taken from it. Its screen drives the display at the same 12 MHz as Bump It, so there was no speed to gain there. The rest of Skiff is a different instrument, so nothing else was copied.
- **Left alone on purpose:** the screen's start-up waits weren't shortened, because the display needs them to start reliably and that can only be checked on a real FM-1.

## Install from Windows

1. **Back up first.** If your FM-1 is on the official firmware, save your projects and sounds with M-VAVE's [FM-1 Workbench](https://open.m-vave.com/fm1-workbench). Bump It, like SLOOP, can't open the official firmware's projects. If your FM-1 is on SLOOP or an earlier Bump It, use [SLOOP's editor](https://isod89.github.io/sloop-fm1/webapp/editor/), Projects, then Backup.
2. Save **Bump-It-1.1-installer.html** to your PC and double-click it so it opens in Chrome or Edge.
3. Connect the FM-1 with a data USB cable, not through a hub, and close any other app or tab that uses MIDI.
4. Wait for the page to say "Bump It 1.1", then press **INSTALL**. Allow MIDI access when the browser asks, and wait for "Done". The page says SLOOP in places because it's SLOOP's own installer with Bump It packed inside.
5. The FM-1 restarts on the purple BUMP IT screen. Your settings and projects carry over. Brightness starts at 100%.

The installer checks the firmware's fingerprint before it writes anything. **Bump-It-1.1.fwsc** is the same firmware on its own, for SLOOP's Python installer or the Firmware Hub. You don't need it for the steps above. The firmware's SHA-256 fingerprint is 456f0249bff8f6efe49918a282ea451404c96a5fe8d855706f0a2b03ef4f0150.

## If something goes wrong

- **The screen is too dark to read:** hold HOME, then turn KNOB 4 all the way right. That puts it back to 100%.
- **The install stops partway:** open the installer page again and press INSTALL. An interrupted install picks up where it left off.
- **It doesn't start:** hold **OCT-** while switching on to get SLOOP's USB rescue mode, then install again.
- **To go back to the earlier 1.1 or to SLOOP:** the 1.1 build from before this polish is on the [v1.1-original release](https://github.com/epidex/bump-it/releases/tag/v1.1-original), and 1.0 is on the [v1.0 release](https://github.com/epidex/bump-it/releases/tag/v1.0). SLOOP is on its [releases page](https://github.com/isod89/sloop-fm1/releases). Install either the same way, and your projects carry over. Set brightness back to 100% first: if the screen was dimmed, they show ZOOM as ON, and you'd turn it off in their menu.
- **To go back to the official firmware:** use M-VAVE's updater, M-UPGRADE, with the official FM-1 firmware file from m-vave.com's Downloads page, as SLOOP's guide says. Restore your Workbench backup afterwards.

## How it was checked

- **Build:** the firmware builds with no errors. The program is 568,608 bytes, and the memory check passes at 60,396 of 98,304 bytes of RAM.
- **Screen checks:** all 395 passed, including the new brightness ones. They cover KNOB 4 stepping down and stopping at 15%, OCT+ going back round to 100%, ZOOM and BRIGHTNESS not affecting each other, and an unknown saved value falling back to 100%.
- **Brightness drawing test:** the real screen code runs against a model of the display. At 100%, the picture reaches the screen exactly as before, in one transfer. At every lower level, every pixel comes out at the right darkness and none are corrupted, even when two screen updates run back to back. To prove the test works, I broke the code on purpose and the test caught it.
- **Saving test:** brightness survives a save and a power-on, the web editor's settings restore applies it, and impossible values are refused. A settings file from before this polish loads at 100%.
- **Editor fuzz test:** this test didn't compile in the earlier 1.1 because its test file was missing one line. With that line added, the full fuzz and its three repeat cases now pass.
- **Full test suite:** 64 sections ran with 1,669 checks passed. One section still fails: four audio routines come out over SLOOP's speed estimates. The numbers are identical to the earlier 1.1, which also failed on them, and the audio code wasn't touched.
- **The installer page** opened in a headless Chrome and showed "Bump It 1.1".

## Not tested yet

- Nothing has run on a real FM-1. Whether it boots, how the dimmed levels look on the actual screen, and whether 0.4 seconds is enough for the boot screen are all unproven.
- Dimming adds a little screen work below 100%. It runs in the screen loop, not the audio, but it hasn't been measured on the hardware.
- Nothing was flashed or written to your synth, and the voltages are SLOOP's own.
