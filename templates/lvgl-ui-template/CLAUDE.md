# LVGL UI Template — AI Agent Workflow Guide

This project is a minimal ESP32-S3-BOX-3 + LVGL + Wi-Fi template designed so
an AI coding agent can develop and verify UI changes **without a human
looking at the screen**: the agent edits code, builds/flashes, takes a
screenshot over Wi-Fi, simulates touch taps over Wi-Fi, and reads the
resulting screenshots back to confirm the UI behaves as expected.

## Build / Flash / Monitor

**Always** dot-source `build_esp32.ps1` to activate the ESP-IDF environment
before running `idf.py` — running `idf.py` directly will fail because the
environment is not set up. First copy `idf-env.example.ps1` → `idf-env.ps1`
(and/or `.bat`) and fill in your local ESP-IDF paths; both are gitignored.

| Task | Command |
|------|---------|
| Full build + flash + monitor (one-shot) | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash monitor"` |
| Build only | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1'"` |
| Activate then manual | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py menuconfig"` |
| Flash only (after build) | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash"` |
| Monitor only | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 monitor"` |
| **macOS** flash + monitor | `idf.py -p /dev/cu.usbmodem31401 flash monitor` |

Key rules:
1. Must **dot-source** (`.`) `build_esp32.ps1` so PATH/IDF_PATH etc. persist
   in the current shell.
2. Activate and `idf.py` must run **in the same command** (same process).
3. Port is **COM3** on Windows / `/dev/cu.usbmodem31401` on macOS.
4. If a serial monitor is already running on the port, kill it before
   flashing.

## Wi-Fi configuration

Set `APP_WIFI_SSID` / `APP_WIFI_PASSWORD` via `idf.py menuconfig` →
"LVGL UI Template Configuration" → "Wi-Fi", or add them to a local
`sdkconfig.defaults.local` (not tracked by git, pass via
`-DSDKCONFIG_DEFAULTS`). Without Wi-Fi credentials, the screenshot/input
debug servers never start (they require an IP address).

## Screenshot + synthetic-touch debug loop

This is the core agent workflow for verifying UI changes:

1. **Build + flash + reset, capture the boot log to get the device IP:**
   ```
   python tools/screenshot/read_serial.py COM3 --reset --seconds 15
   ```
   Look for a line like `WIFI: connected, IP: 192.168.x.x`.

2. **Take a screenshot** (TCP port 3333, raw RGB565 → PNG):
   ```
   python tools/screenshot/screenshot.py <ip> --out tools/screenshot/screenshots/main.png
   ```
   Then use the `Read` tool on the PNG to view it.

3. **Simulate a tap** (TCP port 3334):
   ```
   python tools/input/tap.py <ip> tap <x> <y>
   ```
   Supports `tap`, `down`, `move`, `up` for drag gestures.

4. **Edit UI code → rebuild → reflash → screenshot again → compare.**
   Repeat until the screen matches the intended design.

## Coordinate system / layout reference

The display is **320×240**, origin (0,0) at top-left.

- Title bar: `y=0..36`, full width (320px). `ui_theme_create_title_bar()`
  centers a label in it; `ui_theme_create_back_button()` puts a back button
  at the left edge.
- Content area: `y=36..240` (204px tall).
- Main screen (`ui_screen_main.c`) has 3 stacked buttons at `y=50`, `y=112`,
  `y=174`, each 280×52px, horizontally centered (x center = 160).

**Lesson learned (from the smart-buddy project this template was extracted
from):** when computing tap coordinates for icons placed with
`lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -N, 0)`, the icon's *center* is at
`screen_width - N - (icon_width / 2)`, not at `screen_width - N`. Always
double-check by taking a screenshot after a tap that "did nothing" — a few
pixels off is usually the cause.

## Known pitfall: screenshot task stack size

`components/debug_screenshot/src/debug_screenshot.c` allocates a **16KB
PSRAM-backed static stack** for its task
(`xTaskCreateStaticPinnedToCore` + `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`).

**Do not shrink this stack.** `lv_snapshot_take_to_draw_buf()` recurses
through the entire widget tree; on a complex screen a small (e.g. 4KB)
internal-RAM stack overflows and causes a `LoadProhibited` Guru Meditation
panic (this was a real crash hit during development of the original
project). If you add new screens with deep widget nesting, keep this stack
size or increase it — never reduce it back to a default `xTaskCreate(...,
4096, ...)`.
