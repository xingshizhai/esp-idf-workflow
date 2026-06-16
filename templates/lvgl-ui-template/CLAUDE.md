# LVGL UI Template — AI Agent Development & Debugging Guide

This template lets an AI agent develop and verify ESP32 LVGL UIs **autonomously**:
edit code → build → flash → screenshot over Wi-Fi → tap → screenshot → repeat.
No human needs to look at the physical screen for normal UI work.

---

## 1. Supported Hardware

| `APP_BOARD` | Device | Display | Resolution |
|---|---|---|---|
| `esp32s3_box3` *(default)* | ESP32-S3-BOX-3 | SPI ILI9341 | 320×240 |
| `esp32s3_lcd_ev_board` | ESP32-S3-LCD-EV-BOARD-2 | RGB parallel | 800×480 (sub-board 3) or 480×480 (sub-board 2) |

Board is selected by `APP_BOARD` CMake variable, which is auto-detected from `sdkconfig`
at build time (see `CMakeLists.txt`). Change it via:
```
idf.py menuconfig  →  "LVGL UI Template Configuration" → "Target board"
```
After changing board: `idf.py fullclean` then rebuild.

---

## 2. Environment Setup

**Windows**: Always dot-source `build_esp32.ps1` to activate ESP-IDF.
Copy `idf-env.example.ps1` → `idf-env.ps1` and fill in your local paths (gitignored).

The activation and `idf.py` commands **must be in the same `-Command` string**:
```powershell
powershell -NoProfile -Command ". 'D:\path\to\build_esp32.ps1' -ActivateOnly; idf.py -p COM8 flash"
```

**macOS/Linux**: Source the IDF export script directly, then run `idf.py` normally.

### Quick reference (Windows, LCD-EV-BOARD on COM8)

| Task | Command |
|---|---|
| Build | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py build"` |
| Flash | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM8 flash"` |
| Build + Flash | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py build; idf.py -p COM8 flash"` |
| Monitor | `powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM8 monitor"` |

Kill any running serial monitor before flashing (it holds the COM port).

---

## 3. Wi-Fi Configuration

Set SSID/password via menuconfig → "LVGL UI Template Configuration" → "Wi-Fi",
or in a local `sdkconfig.defaults.local` (gitignored). Without Wi-Fi the screenshot
and touch servers never start.

Get the device IP from the boot log:
```
python tools/screenshot/read_serial.py COM8 --reset --seconds 15
```
Look for: `WIFI: connected, IP: 192.168.x.x`

---

## 4. Core Agent Loop: Edit → Flash → Verify

```
┌─────────────────────────────────────────────────────────────┐
│  1. Edit UI source files (components/ui/src/)               │
│  2. Build + flash                                           │
│  3. Wait ~5s for device to boot and connect to Wi-Fi        │
│  4. Take Wi-Fi screenshot → Read PNG → inspect layout       │
│  5. Simulate taps to navigate / interact                    │
│  6. Take screenshot → verify result                         │
│  7. Repeat from 1 until design is correct                   │
└─────────────────────────────────────────────────────────────┘
```

### 4a. Screenshot (TCP port 3333)

```bash
python tools/screenshot/screenshot.py <device-ip> --out tools/screenshot/screenshots/current.png
```

The server returns raw RGB565 data; the script saves a PNG. Use the `Read` tool on the
PNG to view it. The screenshot captures LVGL's software render buffer — it shows the
correct logical UI regardless of hardware display state (see §7 for the implication).

### 4b. Synthetic touch (TCP port 3334)

```bash
python tools/input/tap.py <device-ip> tap <x> <y>
```

Supports four event types: `tap`, `down`, `move`, `up`. Use `down`/`move`/`up` for
drag/swipe gestures. Coordinates are pixels from the top-left corner of the display.

### 4c. Multi-frame motion detection

```bash
python tools/screenshot/check_motion.py <device-ip> --frames 5 --interval 0.4
```

Takes N screenshots at a fixed interval and compares consecutive pairs pixel-by-pixel.
Exit code: `0` = static, `1` = motion detected, `2` = connection error.
Add `--save-frames` to save each frame and an amplified diff image for inspection.

**Use this to verify** that an animation completed, a spinner stopped, or that a
transition has settled before inspecting the final layout.

---

## 5. Coordinate Systems & Layout

### ESP32-S3-BOX-3 (320×240)

```
(0,0)────────────────────(319,0)
  │  Title bar  y=0..35         │
  ├─────────────────────────────┤
  │  Content area  y=36..239    │
  │                             │
(0,239)──────────────────(319,239)
```

- Title bar height: 36px
- `ui_theme_create_title_bar(scr, "Title")` fills the bar with a label centered
- `ui_theme_create_back_button(bar, cb)` places a back button at the left edge (x≈18)

### ESP32-S3-LCD-EV-BOARD-2 sub-board 3 (800×480)

```
(0,0)────────────────────────────────(799,0)
  │  Title bar  y=0..35               │
  ├───────────────────────────────────┤
  │  Content area  y=36..479          │
  │                                   │
(0,479)──────────────────────────────(799,479)
```

Same title bar API; all widths use `LV_PCT(N)` so layouts are resolution-independent.

### Tap coordinate tips

- Center of a `LV_ALIGN_CENTER` object at offset (dx, dy):
  `x = screen_w/2 + dx`, `y = screen_h/2 + dy`
- Center of `LV_ALIGN_RIGHT_MID` at offset (-N, 0):
  `x = screen_w - N - widget_w/2`
- After a tap that "does nothing", take a screenshot — the widget is usually a few
  pixels away from where you tapped.

---

## 6. UI Code Structure

```
components/ui/src/
  ui_manager.c        ← screen stack: push/pop with slide animations
  ui_theme.c          ← palette, title bar, button styles
  ui_screen_main.c    ← main menu (3 buttons + frame counter)
  ui_screen_page1.c   ← labels demo
  ui_screen_page2.c   ← slider demo
  ui_screen_page3.c   ← list demo
```

### Adding a new screen

1. Create `components/ui/src/ui_screen_mypage.c` returning `lv_obj_t *screen_mypage_create(void)`
2. Add `UI_SCREEN_MYPAGE` to the `ui_screen_id_t` enum in `ui_manager.h`
3. Register it in `ui_manager.c` with `screen_mypage_create`
4. Navigate to it with `ui_manager_push(UI_SCREEN_MYPAGE, UI_ANIM_SLIDE_LEFT)`

### Style conventions

- All widths: `LV_PCT(N)` — never hardcoded pixels, so layouts work on both boards
- Fonts available: `lv_font_montserrat_14`, `_16`, `_24` (20 and others are NOT enabled)
- Colors: always use `ui_theme_palette()` — never hardcode hex colors
- Clear `LV_OBJ_FLAG_SCROLLABLE` on containers that should not scroll
- Wrap LVGL calls outside the LVGL task with `lvgl_port_lock()` / `lvgl_port_unlock()`

### Frame counter (built into main screen)

`ui_screen_main.c` shows `frame: N` at the bottom, incrementing every second.
Use this to confirm that a fresh screenshot reflects live state — compare the
counter value across two screenshots to prove they were taken at different times.

---

## 7. Screenshot Reliability: What It Can and Cannot Detect

### What Wi-Fi screenshots CAN detect ✅

| Can detect | How to verify |
|---|---|
| Wrong widget position / size | Read the PNG, inspect visually |
| Missing or wrong text | Read the PNG |
| Wrong color / style | Read the PNG |
| Navigation worked (page changed) | Screenshot before and after tap |
| Animation completed / spinner stopped | `check_motion.py` — wait until static |
| Screenshot is live (not stale cache) | Check that `frame: N` counter changed |

### What Wi-Fi screenshots CANNOT detect ❌

Wi-Fi screenshots read LVGL's **software render buffer**, not the physical display
hardware. Any artifact produced by the LCD controller or DMA hardware is invisible.

| Cannot detect | Why |
|---|---|
| RGB LCD horizontal scrolling | DMA reads wrong PSRAM offset — LVGL buffer is always correct |
| Display tearing / partial frame | Hardware timing issue below LVGL layer |
| Color channel swap (hardware wiring) | LVGL renders correct colors regardless |
| Backlight off / display physically off | LVGL renders into memory even if nothing is shown |

**Experimentally confirmed (2026-06-16):**

| State | `check_motion.py` result | USB camera result |
|---|---|---|
| Bounce buffer ON — display correct | `UI is static` ✓ (0 px changed) | Static, correct ✓ |
| Bounce buffer OFF — display scrolling | **`UI is static`** ✗ (0 px changed) | **MOVING** ✗ (10–25% changed) |

When the physical screen scrolled continuously, `check_motion.py` reported zero motion
because LVGL's buffer was perfectly correct throughout.

### When to involve a physical camera or the user

If you suspect a hardware display problem (scrolling, black screen, garbled pixels)
and cannot explain it from the LVGL screenshot alone:
1. Ask the user to look at the physical screen, **or**
2. Use a USB camera pointed at the screen — see §8 for the camera motion check script

---

## 8. USB Camera Motion Check (hardware-level verification)

When a USB camera is connected and aimed at the screen, use OpenCV to capture real
physical-display frames and detect motion:

```python
import cv2, time, numpy as np

# USB camera is typically index 1 (index 0 = built-in laptop camera)
cap = cv2.VideoCapture(1, cv2.CAP_DSHOW)   # Windows; omit CAP_DSHOW on Linux/macOS
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
for _ in range(8): cap.read()              # warm-up + exposure settle

frames = []
for i in range(6):
    ret, frame = cap.read()
    if ret: frames.append(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
    if i < 5: time.sleep(0.4)
cap.release()

for i in range(len(frames) - 1):
    delta = np.abs(frames[i].astype(np.int16) - frames[i+1].astype(np.int16)).max(axis=2)
    pct = (delta > 15).sum() / delta.size * 100
    print(f"frame{i+1}→frame{i+2}: {pct:.1f}%  {'MOVING' if pct >= 1.0 else 'static'}")
```

Camera noise produces < 0.7% pixel change. Screen scrolling produces > 5%.

**Dependencies**: `pip install opencv-python numpy`

---

## 9. RGB LCD Hardware Troubleshooting (LCD-EV-BOARD-2 only)

### Symptom: display continuously scrolls horizontally

**Root cause**: PSRAM bandwidth contention. The RGB LCD DMA reads the frame buffer from
PSRAM at the same time the CPU accesses PSRAM (code, data, Wi-Fi buffers). The DMA
stalls and misses its timing window, causing the hardware to start the next scan line
from a wrong byte offset. The entire frame appears shifted left and wraps around.

**Fix**: bounce buffer mode. The DMA reads from a small SRAM buffer; a background task
refills it from PSRAM. CPU and DMA no longer compete.

Required sdkconfig settings:
```
CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_MODE=y
CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_HEIGHT=10
CONFIG_BSP_LCD_RGB_BUFFER_NUMS=1
```

These are pre-set in `sdkconfig.defaults.esp32s3_lcd_ev_board`.

**Do NOT try to fix scrolling with** `avoid_tearing=true`, `direct_mode=true`, or
`full_refresh=true` — those flags address buffer-swap timing between LVGL and the panel
driver, not the underlying PSRAM bandwidth contention. They have no effect on the scroll.

**Do NOT trust Wi-Fi screenshots to confirm the fix** — the scroll is invisible to
`lv_snapshot_take_to_draw_buf()`. Use the USB camera or physical inspection.

### Symptom: crash / abort on startup with `avoid_tearing=true`

`lvgl_port` calls `esp_lcd_rgb_panel_get_frame_buffer(panel, 2, ...)` which requires
`CONFIG_BSP_LCD_RGB_BUFFER_NUMS=2`. With only 1 buffer the call fails and the firmware
aborts. Set `CONFIG_BSP_LCD_RGB_BUFFER_NUMS=2` in sdkconfig if using avoid_tearing mode
(but prefer bounce buffer instead — see above).

### Symptom: build error `-Werror=use-after-free` in `esp_lcd_gc9503`

Upstream component bug (pointer used in `ESP_LOGD` after `free()`). Already suppressed
globally in `CMakeLists.txt` with `-Wno-use-after-free`.

---

## 10. Known Pitfalls

### Screenshot task stack overflow

`debug_screenshot.c` uses a 16 KB PSRAM-backed stack.
`lv_snapshot_take_to_draw_buf()` recurses through the full widget tree — on complex
screens a 4 KB stack causes a `LoadProhibited` Guru Meditation crash.
**Never reduce the stack size below 16 KB.**

### CMakeLists.txt reads `sdkconfig` directly

CMake reads the text `sdkconfig` file (not `sdkconfig.cmake`) to detect the active board
before `include(project.cmake)`. `sdkconfig.cmake` is generated during the build and is
one cycle stale after a menuconfig change — reading it would link the wrong BSP.

### `lv_font_montserrat_20` is not enabled

Only `_14`, `_16`, and `_24` are enabled in sdkconfig. Using `_20` causes a linker error.
Use `_24` as the closest substitute.

### Board switch requires `idf.py fullclean`

CMake caches the BSP component selection. After switching boards via menuconfig, run
`idf.py fullclean` before the next build, otherwise the wrong BSP is linked.
