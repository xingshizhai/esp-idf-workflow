# LVGL UI Template

[中文文档](README.zh-CN.md)

A minimal, clone-and-go multi-board LVGL + Wi-Fi project template, designed so
an AI coding agent (or a human) can develop and verify UI changes **without
looking at the physical screen**: edit code → build/flash → take a screenshot
over Wi-Fi → simulate a touch tap over Wi-Fi → read the screenshot back to
confirm the result.

See [CLAUDE.md](CLAUDE.md) for the full AI-agent workflow guide.

## Supported boards

Board selection is done via **`idf.py menuconfig`** (no command-line flags needed):

> `LVGL UI Template Configuration` → `Target board`

| Board | `menuconfig` option | Display | Touch |
|---|---|---|---|
| **ESP32-S3-BOX-3** *(default)* | `ESP32-S3-BOX-3` | 320×240 SPI (ILI9341) | TT21100 / GT911 |
| **ESP32-S3-LCD-EV-BOARD-2** | `ESP32-S3-LCD-EV-BOARD-2` | 480×480 or 800×480 RGB | FT5x06 / GT1151 |
| **Custom board** | `Custom board (user-defined)` | User-defined | User-defined |

After changing the board, run `idf.py fullclean` before rebuilding.

---

## What's included

- **LVGL UI skeleton** — a main screen with 3 buttons, each pushing onto a
  secondary screen (text demo / slider / scrollable list)
- **Wi-Fi** (`wifi_manager`) — connects on boot, credentials set via menuconfig
- **On-demand screenshot server** (`debug_screenshot`, TCP port 3333) —
  captures the current LVGL framebuffer as raw RGB565 for the PC-side tool to
  convert to PNG
- **Synthetic touch input server** (`debug_input`, TCP port 3334) — injects
  LVGL pointer events (tap / down / move / up) without a real touchscreen
- **PC-side Python tools** (`tools/`) — `screenshot.py`, `read_serial.py`,
  `tap.py`

---

## Getting started

### 1. Set up the ESP-IDF environment

Copy `idf-env.example.ps1` → `idf-env.ps1` (and/or `idf-env.example.bat` →
`idf-env.bat`) and fill in your local ESP-IDF paths. Both files are gitignored.

### 2. Select your board

```powershell
. .\build_esp32.ps1 -ActivateOnly
idf.py menuconfig
```

Navigate to **"LVGL UI Template Configuration" → "Target board"** and pick
your board. For **ESP32-S3-LCD-EV-BOARD-2**, also set the sub-board under
"Board Support Package" → "LCD" → "Select Target Sub board":
- Sub-board 2: 480×480 RGB (GC9503CV + FT5x06)
- Sub-board 3: 800×480 RGB (ST7262E43 + GT1151)

Then set Wi-Fi credentials under **"LVGL UI Template Configuration" → "Wi-Fi"**.

### 3. Build, flash, monitor

```powershell
# First time after changing board: clean first
idf.py fullclean

powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash monitor"
```

> **First build only**: the ESP Component Registry is contacted to download
> the BSP component. Requires internet access. Once `managed_components/` is
> populated, subsequent builds work offline.

### 4. Screenshot + tap debug loop

```powershell
# Get the device IP from the boot log
python tools/screenshot/read_serial.py COM3 --reset --seconds 15

# Take a screenshot
python tools/screenshot/screenshot.py <ip> --out tools/screenshot/screenshots/main.png

# Simulate a tap
python tools/input/tap.py <ip> tap <x> <y>
```

---

## Board-specific notes

### ESP32-S3-BOX-3

- Display: **320×240**, SPI interface, backlight via PWM
- Touch: TT21100 (v1/v2) or GT911
- IMU: ICM-42670-P (used by the upstream BSP, not wired in this template)
- Component: [`espressif/esp-box-3`](https://components.espressif.com/components/espressif/esp-box-3)

### ESP32-S3-LCD-EV-BOARD-2

- Display: **480×480** (sub-board 2) or **800×480** (sub-board 3), RGB parallel interface
- Touch: FT5x06 (sub-board 2) or GT1151 (sub-board 3)
- **No software backlight control** — the RGB panel drives its own backlight
- LVGL draw buffers are allocated in **PSRAM** (RGB panels require it)
- Component: [`espressif/esp32_s3_lcd_ev_board`](https://components.espressif.com/components/espressif/esp32_s3_lcd_ev_board)

### Custom board

Copy `components/boards/custom/` to `components/boards/<your_name>/` and fill
in the TODOs in `board_display.c`, `board_touch.c`, `CMakeLists.txt`, and
`idf_component.yml`. Then:

1. `idf.py menuconfig` → Target board = **Custom board** → set directory name
2. `idf.py fullclean && idf.py build`

The custom board skeleton implements the same `hal_display_create` /
`hal_touch_create` interface and works with all existing UI, Wi-Fi, screenshot,
and touch-injection components without any changes.

---

## Project structure

```
lvgl-ui-template/
├── CMakeLists.txt                    # board selection (Kconfig + optional -DAPP_BOARD)
├── sdkconfig.defaults                # PSRAM, LVGL snapshot, debug servers
├── sdkconfig.defaults.esp32s3_lcd_ev_board  # LCD-EV-BOARD-2 BSP overrides
├── partitions.csv
├── main/
│   ├── main.c                        # init HAL → UI → Wi-Fi → debug servers
│   └── Kconfig.projbuild             # Target board choice + Wi-Fi + debug options
├── components/
│   ├── board_hal/                    # generic HAL interfaces (display/touch/storage)
│   ├── boards/
│   │   ├── esp32s3_box3/             # BOX-3 HAL impl (SPI display, BSP-backed)
│   │   ├── esp32s3_lcd_ev_board/     # LCD-EV-BOARD-2 HAL impl (RGB display)
│   │   └── custom/                   # skeleton for user-defined boards
│   ├── ui/                           # ui_theme, ui_manager, demo screens
│   ├── wifi_manager/
│   ├── debug_screenshot/             # TCP 3333 — LVGL snapshot server
│   └── debug_input/                  # TCP 3334 — synthetic touch server
└── tools/
    ├── screenshot/                   # screenshot.py, read_serial.py
    └── input/                        # tap.py
```

## License

MIT
