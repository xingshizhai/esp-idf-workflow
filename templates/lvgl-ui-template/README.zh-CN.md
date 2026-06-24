# LVGL UI 模板项目

[English](README.md)

一个最小化、clone 即用的多开发板 LVGL + WiFi 项目模板，专为 AI 编程 agent
（或人类开发者）设计：可以在**不看实体屏幕**的情况下开发和验证 UI 改动——改代
码 → 编译烧录 → 通过 WiFi 截图 → 通过 WiFi 模拟点击 → 读取截图确认结果。

完整的 AI agent 工作流说明见 [CLAUDE.md](CLAUDE.md)。

## 支持的开发板

板卡通过 **`idf.py menuconfig`** 选择（无需命令行参数）：

> `LVGL UI Template Configuration` → `Target board`

| 开发板 | menuconfig 选项 | 屏幕 | 触摸 |
|---|---|---|---|
| **ESP32-S3-BOX-3** *（默认）* | `ESP32-S3-BOX-3` | 320×240 SPI (ILI9341) | TT21100 / GT911 |
| **ESP32-S3-LCD-EV-BOARD-2** | `ESP32-S3-LCD-EV-BOARD-2` | 480×480 或 800×480 RGB | FT5x06 / GT1151 |
| **自定义板卡** | `Custom board (user-defined)` | 用户定义 | 用户定义 |

切换板卡后，须执行 `idf.py fullclean` 再重新编译。

---

## 包含内容

- **LVGL UI 骨架** —— 主界面 3 个按钮，分别跳转到 3 个二级界面（文本展示 / 滑条 / 可滚动列表）
- **WiFi**（`wifi_manager`）—— 开机自动连接，账号密码通过 menuconfig 配置
- **按需截图服务**（`debug_screenshot`，TCP 3333 端口）—— 抓取当前 LVGL
  帧缓冲，以 RGB565 原始数据发送，由 PC 端工具转为 PNG
- **合成触摸输入服务**（`debug_input`，TCP 3334 端口）—— 注入 LVGL pointer
  事件（tap / down / move / up），无需真实触摸屏即可驱动 UI 流程
- **PC 端 Python 工具**（`tools/`）—— `screenshot.py`、`read_serial.py`、`tap.py`

---

## 快速开始

### 1. 配置 ESP-IDF 环境

将 `idf-env.example.ps1` 复制为 `idf-env.ps1`（需 CMD 支持则同时复制
`idf-env.example.bat` → `idf-env.bat`），填入本机的 ESP-IDF 路径。两个文件
均已加入 `.gitignore`，不会被提交。

### 2. 选择板卡并配置

```powershell
. .\build_esp32.ps1 -ActivateOnly
idf.py menuconfig
```

进入 **"LVGL UI Template Configuration" → "Target board"**，选择目标板卡。

若选择 **ESP32-S3-LCD-EV-BOARD-2**，还需在
"Board Support Package" → "LCD" → "Select Target Sub board" 选择子板：
- Sub-board 2：480×480 RGB（GC9503CV + FT5x06 触摸）
- Sub-board 3：800×480 RGB（ST7262E43 + GT1151 触摸）

同时在 **"LVGL UI Template Configuration" → "Wi-Fi"** 填写 WiFi 账号密码。

### 3. 编译、烧录、监控

```powershell
# 切换过板卡后需先清理
idf.py fullclean

powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash monitor"
```

> **首次编译说明**：组件管理器会从 ESP 组件仓库下载 BSP，需要网络连接。
> 一旦 `managed_components/` 目录生成后，后续编译可离线进行。

### 4. 截图 + 点击调试闭环

```powershell
# 从启动日志中获取设备 IP
python tools/screenshot/read_serial.py COM3 --reset --seconds 15

# 截图
python tools/screenshot/screenshot.py <ip> --out tools/screenshot/screenshots/main.png

# 模拟点击
python tools/input/tap.py <ip> tap <x> <y>
```

---

## 各板卡说明

### ESP32-S3-BOX-3

- 屏幕：**320×240**，SPI 接口，支持 PWM 背光调节
- 触摸：TT21100（v1/v2）或 GT911
- 组件：[`espressif/esp-box-3`](https://components.espressif.com/components/espressif/esp-box-3)

### ESP32-S3-LCD-EV-BOARD-2

- 屏幕：**480×480**（子板 2）或 **800×480**（子板 3），RGB 并行接口
- 触摸：FT5x06（子板 2）或 GT1151（子板 3）
- **不支持软件背光控制**——RGB 面板自驱背光
- LVGL 绘制缓冲区分配在 **PSRAM**（RGB 接口所需）
- 组件：[`espressif/esp32_s3_lcd_ev_board`](https://components.espressif.com/components/espressif/esp32_s3_lcd_ev_board)

### 自定义板卡

将 `components/boards/custom/` 复制到 `components/boards/<你的板卡名>/`，
然后按 TODO 注释填写：

- `board_display.c` —— LCD 驱动初始化（SPI / RGB / MIPI 均可）
- `board_touch.c` —— 触摸控制器初始化（无触摸则保持 `tp = NULL`）
- `CMakeLists.txt` —— 添加所需驱动组件
- `idf_component.yml` —— 声明外部依赖

配置步骤：

```
1. idf.py menuconfig
   → Target board = Custom board (user-defined)
   → Custom board directory name = <你的板卡名>

2. idf.py fullclean && idf.py build
```

自定义骨架实现的 `hal_display_create` / `hal_touch_create` 接口与其他所有
组件（UI、WiFi、截图、合成触摸）完全兼容，无需任何额外改动。

---

## 项目结构

```
lvgl-ui-template/
├── CMakeLists.txt                          # 板卡选择（Kconfig + 可选 -DAPP_BOARD）
├── sdkconfig.defaults                      # PSRAM、LVGL snapshot、调试服务
├── sdkconfig.defaults.esp32s3_lcd_ev_board # LCD-EV-BOARD-2 BSP 默认配置
├── partitions.csv
├── main/
│   ├── main.c                              # 初始化 HAL → UI → WiFi → 调试服务
│   └── Kconfig.projbuild                   # 板卡选择 + WiFi + 调试服务选项
├── components/
│   ├── board_hal/                          # 通用 HAL 接口（display/touch/storage）
│   ├── boards/
│   │   ├── esp32s3_box3/                   # BOX-3 HAL 实现（SPI 屏，BSP 驱动）
│   │   ├── esp32s3_lcd_ev_board/           # LCD-EV-BOARD-2 HAL 实现（RGB 屏）
│   │   └── custom/                         # 自定义板卡骨架（复制后填写 TODO）
│   ├── ui/                                 # ui_theme、ui_manager、示例界面
│   ├── wifi_manager/
│   ├── debug_screenshot/                   # TCP 3333 截图服务
│   └── debug_input/                        # TCP 3334 合成触摸服务
└── tools/
    ├── screenshot/                         # screenshot.py、read_serial.py
    └── input/                              # tap.py
```

## License

MIT
