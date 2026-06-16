# LVGL UI Template — AI Agent Workflow Guide

This project is a multi-board LVGL + Wi-Fi template designed so an AI coding
agent can develop and verify UI changes **without a human looking at the
screen**: the agent edits code, builds/flashes, takes a screenshot over Wi-Fi,
simulates touch taps over Wi-Fi, and reads the resulting screenshots back to
confirm the UI behaves as expected.

Supported boards (selected at build time via `APP_BOARD`):

| `APP_BOARD` | Hardware | Display |
|---|---|---|
| `esp32s3_box3` *(default)* | ESP32-S3-BOX-3 | 320×240 SPI (ILI9341) |
| `esp32s3_lcd_ev_board` | ESP32-S3-LCD-EV-BOARD-2 | 480×480 or 800×480 RGB |

## Build / Flash / Monitor

**Always** dot-source `build_esp32.ps1` to activate the ESP-IDF environment
before running `idf.py` — running `idf.py` directly will fail because the
environment is not set up. First copy `idf-env.example.ps1` → `idf-env.ps1`
(and/or `.bat`) and fill in your local ESP-IDF paths; both are gitignored.

### ESP32-S3-BOX-3 (default board)

| Task | Command |
|------|---------|
| Full build + flash + monitor | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash monitor"` |
| Build only | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1'"` |
| menuconfig | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py menuconfig"` |
| Flash only | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash"` |
| Monitor only | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 monitor"` |
| **macOS** flash + monitor | `idf.py -p /dev/cu.usbmodem31401 flash monitor` |

### ESP32-S3-LCD-EV-BOARD-2

Pass `-DAPP_BOARD=esp32s3_lcd_ev_board` to CMake (or set it in `build_esp32.ps1`).
**Board selection via menuconfig:**
```
idf.py menuconfig  →  "LVGL UI Template Configuration" → "Target board"
```
After changing the board, run `idf.py fullclean` then rebuild so CMake links
the correct BSP component.

Sub-board selection (480×480 vs 800×480) is under menuconfig →
"Board Support Package" → "LCD" → "Select Target Sub board".

The file `sdkconfig.defaults.esp32s3_lcd_ev_board` pre-sets reasonable defaults
(800×480 sub-board 3) when the sdkconfig is first created.  After switching
boards you can also pass `-DAPP_BOARD=esp32s3_lcd_ev_board` on the command
line to override Kconfig for one-off builds.

**Important LCD-EV-BOARD-2 caveats:**
- The RGB panel has **no software backlight control** — `hal_display_t.backlight_set` is NULL.
- `BSP_LCD_H_RES` / `BSP_LCD_V_RES` are runtime function calls, not compile-time constants.
- The first-time build downloads the BSP component from the Espressif component registry; an internet connection is required.
- **Bounce buffer mode is required** (`CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_MODE=y`).
  Without it the LCD DMA reads the PSRAM frame buffer directly, competing with the CPU
  for PSRAM bandwidth and causing the entire display to scroll horizontally.
  Bounce buffer mode routes the DMA through a small SRAM intermediate buffer, eliminating
  the contention. `avoid_tearing` / `direct_mode` / `full_refresh` flags in lvgl_port do
  NOT fix this — they address buffer-swap timing, not the PSRAM bandwidth root cause.

**Screenshot coordinates:**
- Sub-board 2: **480×480**, origin (0,0) at top-left.
- Sub-board 3: **800×480**, origin (0,0) at top-left.

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

## Wi-Fi screenshot limitation: cannot detect hardware-layer scrolling

`lv_snapshot_take_to_draw_buf()` reads from LVGL's **software render buffer** — it captures
what LVGL *thinks* the screen looks like, not the actual pixels output by the hardware.

**Experimentally verified (2026-06-16):**

| Condition | Wi-Fi screenshot result | Camera (physical screen) |
|---|---|---|
| bounce buffer ON (normal) | frame counter changes ✓ | static, correct layout ✓ |
| bounce buffer OFF (scroll bug) | **"UI is static"** ✗ | MOVING — horizontal scroll ✗ |

When the RGB LCD has PSRAM bandwidth contention causing the DMA to shift the frame:
- Wi-Fi `check_motion.py` reports **0 px changed** — completely blind to the problem
- USB camera multi-frame diff shows **10–25% pixels changing** — correctly detects it

**Conclusion for AI agents**: Wi-Fi screenshots are reliable for verifying UI layout,
navigation, widget state, and text content. They are **not** reliable for detecting
hardware-layer display artifacts (tearing, scrolling, colour corruption). When an RGB
LCD displays incorrectly and the cause is unknown, use a physical camera or ask the user
to visually confirm. The `frame: N` counter in the main screen can be used to confirm
that screenshots reflect live state (not a stale cache).

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
