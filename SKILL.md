---
name: esp-idf-workflow
description: >
  ESP-IDF development workflow for ESP32 / ESP32-S3 / ESP32-C3 firmware.
  Use when: (1) activating ESP-IDF environment, (2) building firmware,
  (3) flashing firmware to device, (4) serial monitor/debugging, or
  (5) running any idf.py command. Supports Windows (CMD, PowerShell),
  Linux, and macOS. Handles cross-platform environment activation,
  build/flash/monitor workflows, and Windows-specific serial port considerations.
---

## Platform Detection

Detect environment before choosing commands:

| Platform | Shell | Activation Script |
|----------|-------|-------------------|
| Windows | PowerShell | `$IDF_PATH/export.ps1` or EIM activationScript |
| Windows | CMD | `%IDF_PATH%\export.bat` |
| Linux/macOS | bash/zsh | `$IDF_PATH/export.sh` |

**Default ESP-IDF install path on Windows**: `C:\Espressif\esp-idf\` or via EIM at `D:\Tools\esp\.espressif\v<version>\esp-idf`.

## Environment Activation

ESP-IDF requires running an export script before `idf.py` is available. Activation and subsequent commands must run in the **same shell process**.

### Config File Discovery

Config files define ESP-IDF paths. Search order:
1. `$env:ESP_IDF_BUILD_CONFIG` env var (full path to .ps1/.bat)
2. `idf-env.ps1` / `idf-env.bat` in project root
3. `$HOME/.esp-idf-build.ps1` / `~/.esp-idf-build.bat`

Required variable: `$IDF_PATH` / `%IDF_PATH%`
Optional: `$IDF_TOOLS_PATH`, `$IDF_PYTHON_ENV_PATH`

### Activation by Platform

**PowerShell (dot-source — environment persists in current session):**
```powershell
. .\build_esp32.ps1 -ActivateOnly
idf.py build
```

**CMD (batch wrapper delegates to PowerShell):**
```cmd
build_esp32.bat
```

**One-liner for automation/CI (same process):**
```powershell
powershell -NoProfile -Command ". '<abs-path>\build_esp32.ps1' -ActivateOnly; idf.py build"
```

**Linux/macOS:**
```bash
. $IDF_PATH/export.sh
idf.py build
```

### Windows Activation Details

The project's `build_esp32.ps1` handles:
1. Loading config (`idf-env.ps1`)
2. EIM activation check (`%IDF_TOOLS_PATH%\eim_idf.json`) — if IDF_PATH matches, dot-sources the EIM activationScript
3. Fallback to `$IDF_PATH/export.ps1`
4. Cleaning MSYS/MINGW env vars that trigger spurious warnings
5. Switching to project directory

**Critical**: `-ActivateOnly` uses `return` not `exit`, so it can be chained in a single `-Command` string.

## Workflow Commands

All commands below assume environment is activated and working directory is the project root.

### Build
```
idf.py build
```

### Flash
```
idf.py -p <PORT> flash
```

### Build + Flash
```
idf.py -p <PORT> build flash
```

### Serial Monitor
```
idf.py -p <PORT> monitor
```

### Build + Flash + Monitor (full workflow)
```
idf.py -p <PORT> build flash monitor
```

### Configuration UI
```
idf.py menuconfig
```

### Clean Build
```
idf.py fullclean
idf.py build
```

### Set Chip Target
```
idf.py set-target esp32s3    # ESP32-S3
idf.py set-target esp32c3    # ESP32-C3
idf.py set-target esp32      # ESP32
```
After `set-target`, run `fullclean` then `build`.

### Other Useful Subcommands
| Command | Purpose |
|---------|---------|
| `idf.py size` | Show partition sizes |
| `idf.py size-files` | Show per-component sizes |
| `idf.py app` | Build app only (not bootloader/partition table) |
| `idf.py bootloader` | Build bootloader only |

## Serial Port Handling

### Auto-Detect Ports

**PowerShell:**
```powershell
[System.IO.Ports.SerialPort]::GetPortNames()
```

**CMD:**
```cmd
mode
```

**Linux/macOS:**
```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

### Port Naming by Platform

| Platform | Format | Examples |
|----------|--------|----------|
| Windows | `COM<n>` | `COM3`, `COM8` |
| Linux | `/dev/ttyUSB<n>`, `/dev/ttyACM<n>` | `/dev/ttyUSB0` |
| macOS | `/dev/cu.usbserial-*`, `/dev/cu.usbmodem*` | `/dev/cu.usbserial-110` |

### Windows Serial Port Gotchas

1. **Naming**: Always `COM<n>`, never `/dev/` paths
2. **Drivers**: ESP32-S3 has built-in USB-JTAG; older boards use CP210x or CH340 bridges — ensure driver installed
3. **Port locking**: Windows locks COM ports — if monitor fails with "Access denied", close other terminals/IDEs using the port
4. **Baud rate**: Default 921600; if unstable, use `--baud 115200`
5. **Boot mode**: If flash fails with "Failed to connect", hold BOOT button while connecting, then flash
6. **USB-JTAG vs UART**: ESP32-S3 USB-C port may appear as two COM ports — one for UART (serial), one for USB-JTAG (flashing/debugging). Use the UART port for `monitor`, either works for `flash`

## Cross-Platform Automation Template

For scripts that must work on all platforms:

```bash
#!/bin/bash
# Works in Git Bash on Windows, Linux, macOS

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

case "$(uname -s)" in
    CYGWIN*|MINGW*|MSYS*)
        # Windows — delegate to PowerShell
        powershell -NoProfile -Command ". '$PROJECT_DIR/build_esp32.ps1' -ActivateOnly; idf.py $*"
        ;;
    Darwin*)
        . "$IDF_PATH/export.sh"
        idf.py "$@"
        ;;
    Linux*)
        . "$IDF_PATH/export.sh"
        idf.py "$@"
        ;;
esac
```

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `idf.py: command not found` | Environment not activated | Run export script or build_esp32.ps1 first |
| Flash "Failed to connect" | Wrong COM port / not in boot mode | Verify port, hold BOOT button |
| Flash "Access denied" | Port locked by another process | Close other terminals using the port |
| "MSys/Mingw" warnings | MSYS2 env vars leaking | Run `Remove-Item Env:MSYSTEM` etc. |
| CMake errors | Stale build directory | `idf.py fullclean` then rebuild |
| Build slow | No ccache | Install ccache, add to PATH |

## Project Structure

```
project/
├── main/                 # Firmware source
│   ├── main.c            # Entry point
│   ├── CMakeLists.txt
│   └── ...
├── managed_components/   # Managed dependencies
├── build/                # Build output (gitignored)
├── CMakeLists.txt        # Project CMake
├── sdkconfig             # ESP-IDF config (gitignored)
├── sdkconfig.defaults    # Config defaults
├── dependencies.lock     # Dependency versions
├── build_esp32.ps1       # PowerShell build+flash (Windows)
├── build_esp32.bat       # CMD wrapper (Windows)
├── do_flash.bat          # Quick flash script (Windows)
├── idf-env.ps1           # ESP-IDF paths (Windows, gitignored)
├── idf-env.bat           # CMD paths (Windows, gitignored)
├── serial_monitor.py     # Python serial monitor (optional)
└── CLAUDE.md             # Project documentation
```
