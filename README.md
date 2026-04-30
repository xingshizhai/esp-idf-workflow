# ESP-IDF Workflow

[English](#english) | [中文](#中文)

A Claude Code skill that provides a complete development workflow for ESP32 / ESP32-S3 / ESP32-C3 firmware — environment activation, build, flash, and serial monitor — across Windows (CMD, PowerShell), Linux, and macOS.

---

## English

### Why This Skill

ESP-IDF on Windows is notoriously painful:

- **Environment activation** requires dot-sourcing `export.ps1` in the **same shell process** — forget it and `idf.py` is not found
- **EIM (Espressif Install Manager)** has its own activation path, different from the bundled `export.ps1`
- **MSYS/MINGW env vars** leak and cause spurious warnings during build
- **Serial ports** use `COM<n>` naming (not `/dev/ttyUSB0`), and Windows locks them — "Access denied" is a daily occurrence
- **USB-JTAG vs UART**: ESP32-S3 exposes two COM ports on a single USB-C cable, easy to pick the wrong one for serial monitor
- **Batch + PowerShell**: CMD users need a wrapper; PowerShell users need dot-source syntax; automation tools need one-liners — all different

This skill solves all of the above. It gives Claude Code a structured, cross-platform workflow so that AI-assisted ESP-IDF development just works.

### What It Does

| Feature | Description |
|---------|-------------|
| **Environment Activation** | Auto-detects config (`idf-env.ps1`/`idf-env.bat`), handles EIM or `export.ps1` activation, cleans MSYS vars |
| **Build** | `idf.py build` — compiles firmware, bootloader, and partition table |
| **Flash** | `idf.py -p COM8 flash` — writes firmware to device |
| **Serial Monitor** | `idf.py -p COM8 monitor` — real-time log output |
| **Auto-detect COM Ports** | Python script scans for CP210x/CH340/USB-JTAG bridges |
| **Cross-Platform** | Windows (PowerShell + CMD), Linux, macOS — one skill covers all |

### Quick Start

#### 1. Install as a Claude Code Skill

```bash
# Local install (all projects)
# Place the skill directory into ~/.claude/skills/
# On Windows: C:\Users\<username>\.claude\skills\esp-idf-workflow

# Or install from a .skill file
# (Claude Code reads skills from the global skills directory automatically)
```

#### 2. Set Up Your ESP-IDF Project

Create `idf-env.ps1` in your project root with your ESP-IDF paths:

```powershell
$IDF_PATH = "D:\Tools\esp\.espressif\v6.0\esp-idf"
$IDF_TOOLS_PATH = "C:\Espressif\tools"
$IDF_PYTHON_ENV_PATH = "C:\Espressif\tools\python\v6.0\venv"
```

Also create `idf-env.bat` for CMD support:

```bat
set "IDF_PATH=D:\Tools\esp\.espressif\v6.0\esp-idf"
set "IDF_TOOLS_PATH=C:\Espressif\tools"
set "IDF_PYTHON_ENV_PATH=C:\Espressif\tools\python\v6.0\venv"
```

#### 3. Use with Claude Code

Once the skill is installed, Claude Code automatically loads it when you mention ESP-IDF tasks:

- "Build the firmware"
- "Flash to COM8"
- "Show me the serial monitor output"
- "Set the target to esp32s3 and rebuild"

Claude will follow the correct workflow: activate environment → run `idf.py` → handle errors.

### Windows-Specific Features

#### EIM Activation

If you use the Espressif Install Manager (EIM), the workflow automatically detects `eim_idf.json` and uses the correct activation script — no manual `export.ps1` needed.

#### MSYS/MINGW Warning Cleanup

ESP-IDF build warns about MSYS2 env vars (`MSYSTEM`, `MINGW_PREFIX`, `MINGW_CHOST`). The activation script automatically clears them from the current process, so your build output stays clean.

#### Serial Port Detection

Windows-specific COM port detection script:

```powershell
# Auto-detect ESP32 serial ports
python scripts/list_serial_ports.py
```

Output example:
```
Found 9 serial port(s):
--------------------------------------------------
  2. COM8
     Description: Silicon Labs CP210x USB to UART Bridge (COM8)
     Hardware ID: USB VID:PID=10C4:EA60 SER=FE58AB6DF21AEF1181E911764909FFD0
```

#### Serial Monitor with Auto-Reconnect

```powershell
python scripts/serial_monitor.py -p COM8 --filter "ERROR|WARN" --log monitor.log
```

Features:
- Regex-based line filtering (`--filter`)
- Log file output (`--log`)
- Auto-reconnect on USB disconnect (`--reconnect-delay`)
- Works with any baud rate (`-b 115200`)

### Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `idf.py: command not found` | Environment not activated | Run activation script first |
| Flash "Failed to connect" | Wrong COM port / not in boot mode | Verify port, hold BOOT button while connecting |
| Flash "Access denied" | Port locked by another process | Close other terminals/IDEs using the port |
| "MSys/Mingw" warnings | MSYS2 env vars leaking | Activation script clears them automatically |
| CMake errors | Stale build directory | `idf.py fullclean` then rebuild |

### Project Structure

```
your-project/
├── main/                 # Firmware source
├── build/                # Build output (gitignored)
├── CMakeLists.txt        # Project CMake
├── sdkconfig             # ESP-IDF config (gitignored)
├── idf-env.ps1           # ESP-IDF paths (Windows PowerShell)
├── idf-env.bat           # ESP-IDF paths (Windows CMD)
├── build_esp32.ps1       # PowerShell build script
├── build_esp32.bat       # CMD wrapper
└── CLAUDE.md             # Project documentation
```

### Supported Chips

- **ESP32** — Original ESP32
- **ESP32-S3** — AI/ML focused, with USB-JTAG
- **ESP32-C3** — RISC-V, cost-effective

---

## 中文

### 为什么需要这个技能

Windows 下使用 ESP-IDF 开发 ESP32 非常繁琐：

- **环境激活**必须在**同一 shell 进程**中 dot-source `export.ps1`，否则 `idf.py` 找不到
- **EIM（乐鑫安装管理器）**有自己的激活方式，和自带的 `export.ps1` 不同
- **MSYS/MINGW 环境变量**会泄漏，导致编译时出现大量无意义警告
- **串口**使用 `COM<n>` 命名（不是 `/dev/ttyUSB0`），而且 Windows 会锁定串口——"Access denied" 是家常便饭
- **USB-JTAG vs UART**：ESP32-S3 通过一根 USB-C 线会显示两个 COM 口，串口中很容易选错
- **批处理 + PowerShell**：CMD 用户需要包装脚本，PowerShell 用户需要 dot-source 语法，自动化工具需要一行命令——全部不同

这个技能解决以上所有痛点。它为 Claude Code 提供了结构化、跨平台的工作流，让 AI 辅助的 ESP-IDF 开发顺畅运行。

### 功能一览

| 功能 | 说明 |
|------|------|
| **环境激活** | 自动发现配置文件（`idf-env.ps1`/`idf-env.bat`），处理 EIM 或 `export.ps1` 激活，清理 MSYS 变量 |
| **编译** | `idf.py build` — 编译固件、bootloader 和分区表 |
| **烧录** | `idf.py -p COM8 flash` — 将固件写入设备 |
| **串口监控** | `idf.py -p COM8 monitor` — 实时查看日志输出 |
| **自动检测 COM 口** | Python 脚本自动识别 CP210x/CH340/USB-JTAG 桥接器 |
| **跨平台** | Windows (PowerShell + CMD)、Linux、macOS — 一个技能全覆盖 |

### 快速开始

#### 1. 安装为 Claude Code 技能

```bash
# 本地安装（所有项目可用）
# 将技能目录放入 ~/.claude/skills/
# Windows 路径: C:\Users\<用户名>\.claude\skills\esp-idf-workflow
```

#### 2. 配置你的 ESP-IDF 项目

在项目根目录创建 `idf-env.ps1`，填入你的 ESP-IDF 路径：

```powershell
$IDF_PATH = "D:\Tools\esp\.espressif\v6.0\esp-idf"
$IDF_TOOLS_PATH = "C:\Espressif\tools"
$IDF_PYTHON_ENV_PATH = "C:\Espressif\tools\python\v6.0\venv"
```

同时创建 `idf-env.bat` 以支持 CMD：

```bat
set "IDF_PATH=D:\Tools\esp\.espressif\v6.0\esp-idf"
set "IDF_TOOLS_PATH=C:\Espressif\tools"
set "IDF_PYTHON_ENV_PATH=C:\Espressif\tools\python\v6.0\venv"
```

#### 3. 在 Claude Code 中使用

安装技能后，当你提到 ESP-IDF 相关任务时，Claude Code 会自动加载此技能：

- "编译固件"
- "烧录到 COM8"
- "查看串口输出"
- "设置目标芯片为 esp32s3 并重新编译"

Claude 会按正确流程执行：激活环境 → 运行 `idf.py` → 处理错误。

### Windows 专属特性

#### EIM 自动激活

如果你使用乐鑫安装管理器（EIM），工作流会自动检测 `eim_idf.json` 并使用正确的激活脚本——无需手动运行 `export.ps1`。

#### MSYS/MINGW 警告清理

ESP-IDF 编译时会因 MSYS2 环境变量（`MSYSTEM`、`MINGW_PREFIX`、`MINGW_CHOST`）产生警告。激活脚本会在当前进程中自动清除这些变量，保持编译输出干净。

#### 串口自动检测

Windows 专用的 COM 口检测脚本：

```powershell
# 自动检测 ESP32 串口
python scripts/list_serial_ports.py
```

输出示例：
```
Found 9 serial port(s):
--------------------------------------------------
  2. COM8
     Description: Silicon Labs CP210x USB to UART Bridge (COM8)
     Hardware ID: USB VID:PID=10C4:EA60 SER=FE58AB6DF21AEF1181E911764909FFD0
```

#### 串口监视器（带自动重连）

```powershell
python scripts/serial_monitor.py -p COM8 --filter "ERROR|WARN" --log monitor.log
```

功能：
- 正则表达式行过滤（`--filter`）
- 日志文件输出（`--log`）
- USB 断开后自动重连（`--reconnect-delay`）
- 支持任意波特率（`-b 115200`）

### 常见问题

| 现象 | 原因 | 解决方法 |
|------|------|----------|
| `idf.py: command not found` | 环境未激活 | 先运行激活脚本 |
| 烧录 "Failed to connect" | 端口错误/未进入 boot 模式 | 确认 COM 口，连接时按住 BOOT 按钮 |
| 烧录 "Access denied" | 串口被其他程序占用 | 关闭其他使用串口的终端/IDE |
| "MSys/Mingw" 警告 | MSYS2 环境变量泄漏 | 激活脚本会自动清除 |
| CMake 错误 | 旧的构建产物 | `idf.py fullclean` 后重新编译 |

### 项目结构

```
your-project/
├── main/                 # 固件源码
├── build/                # 构建输出（gitignore）
├── CMakeLists.txt        # 项目 CMake
├── sdkconfig             # ESP-IDF 配置（gitignore）
├── idf-env.ps1           # ESP-IDF 路径（Windows PowerShell）
├── idf-env.bat           # ESP-IDF 路径（Windows CMD）
├── build_esp32.ps1       # PowerShell 编译脚本
├── build_esp32.bat       # CMD 包装脚本
└── CLAUDE.md             # 项目文档
```

### 支持的芯片

- **ESP32** — 原始 ESP32
- **ESP32-S3** — 面向 AI/ML，内置 USB-JTAG
- **ESP32-C3** — RISC-V 架构，高性价比

---

## License

MIT
