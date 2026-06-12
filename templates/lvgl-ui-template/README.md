# LVGL UI Template (ESP32-S3-BOX-3)

[中文文档](README.zh-CN.md)

A minimal, clone-and-go ESP32-S3-BOX-3 + LVGL + Wi-Fi project template, designed
so an AI coding agent (or a human) can develop and verify UI changes **without
looking at the physical screen**: edit code → build/flash → take a screenshot
over Wi-Fi → simulate a touch tap over Wi-Fi → read the screenshot back to
confirm the result.

See [CLAUDE.md](CLAUDE.md) for the full AI-agent workflow guide (build/flash
commands, screenshot/tap debug loop, coordinate-system reference).

## What's included

- **LVGL UI skeleton** — a main screen with 3 buttons, each pushing onto a
  secondary screen:
  - Page 1: static labels / text demo
  - Page 2: a slider with a live value label
  - Page 3: a scrollable list
- **Wi-Fi** (`wifi_manager`) — connects on boot using credentials from
  `idf.py menuconfig`
- **On-demand screenshot server** (`debug_screenshot`, TCP port 3333) —
  captures the current LVGL framebuffer via `lv_snapshot_take_to_draw_buf()`
  and streams it as raw RGB565 for the PC-side tool to convert to PNG
- **Synthetic touch input server** (`debug_input`, TCP port 3334) — injects
  LVGL pointer events (tap / down / move / up) so UI flows can be driven
  without a real touchscreen
- **PC-side Python tools** (`tools/`) — `screenshot.py`, `read_serial.py`,
  `tap.py`

## Hardware

- **Target**: ESP32-S3-BOX-3
- **ESP-IDF**: v6.0.1
- **Display**: 320×240 LCD with capacitive touch

## Getting started

### 1. Set up the ESP-IDF environment

Copy `idf-env.example.ps1` → `idf-env.ps1` (and/or `idf-env.example.bat` →
`idf-env.bat`) and fill in your local ESP-IDF paths. Both `idf-env.ps1` and
`idf-env.bat` are gitignored — they're machine-local configuration.

### 2. Configure Wi-Fi

```powershell
. .\build_esp32.ps1 -ActivateOnly
idf.py menuconfig
```

Go to **"LVGL UI Template Configuration" → "Wi-Fi"** and set
`APP_WIFI_SSID` / `APP_WIFI_PASSWORD`. Without Wi-Fi credentials, the
screenshot/input debug servers never start (they need an IP address).

### 3. Build, flash, monitor

```powershell
powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash monitor"
```

> **First build only**: resolving the `espressif/esp-box-3` component
> dependency requires reaching the ESP Component Registry. If you're behind
> a proxy, set `HTTP_PROXY` / `HTTPS_PROXY` before running `idf.py` — see
> [CLAUDE.md](CLAUDE.md) for details. Once `managed_components/` is
> populated, later builds work offline.

### 4. Screenshot + tap debug loop

```powershell
# Get the device IP from the boot log
python tools/screenshot/read_serial.py COM3 --reset --seconds 15

# Take a screenshot
python tools/screenshot/screenshot.py <ip> --out tools/screenshot/screenshots/main.png

# Simulate a tap
python tools/input/tap.py <ip> tap <x> <y>
```

See [CLAUDE.md](CLAUDE.md) for the full coordinate-system reference and
recommended agent workflow.

## Project structure

```
lvgl-ui-template/
├── CMakeLists.txt              # root CMake, board-selection via APP_BOARD
├── sdkconfig.defaults          # PSRAM + LVGL snapshot + debug servers
├── partitions.csv
├── main/                        # app_main: init HAL, UI, Wi-Fi, debug servers
├── components/
│   ├── board_hal/               # generic display/touch/storage interfaces
│   ├── boards/esp32s3_box3/      # board-specific HAL implementation
│   ├── ui/                       # ui_theme, ui_manager, 4 demo screens
│   ├── wifi_manager/
│   ├── debug_screenshot/         # TCP 3333
│   └── debug_input/               # TCP 3334
└── tools/                        # PC-side Python helpers
```

## License

MIT
