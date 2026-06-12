# LVGL UI 模板项目（ESP32-S3-BOX-3）

[English](README.md)

一个最小化、clone 即用的 ESP32-S3-BOX-3 + LVGL + WiFi 项目模板，专为 AI
编程 agent（或人类开发者）设计：可以在**不看实体屏幕**的情况下开发和验证 UI
改动——改代码 → 编译烧录 → 通过 WiFi 截图 → 通过 WiFi 模拟点击 → 读取截图确认
结果。

完整的 AI agent 工作流说明（编译/烧录命令、截图/点击调试闭环、坐标系参考）见
[CLAUDE.md](CLAUDE.md)。

## 包含内容

- **LVGL UI 骨架** —— 主界面 3 个按钮，分别跳转到 3 个二级界面：
  - Page 1：静态文本/标签展示
  - Page 2：带实时数值标签的滑条
  - Page 3：可滚动列表
- **WiFi**（`wifi_manager`）—— 开机自动连接，账号密码通过 `idf.py menuconfig`
  配置
- **按需截图服务**（`debug_screenshot`，TCP 3333 端口）—— 通过
  `lv_snapshot_take_to_draw_buf()` 抓取当前 LVGL 帧缓冲，以 RGB565 原始数据
  发送，由 PC 端工具转换为 PNG
- **合成触摸输入服务**（`debug_input`，TCP 3334 端口）—— 注入 LVGL pointer
  事件（tap / down / move / up），无需真实触摸屏即可驱动 UI 流程
- **PC 端 Python 工具**（`tools/`）—— `screenshot.py`、`read_serial.py`、
  `tap.py`

## 硬件

- **目标芯片**：ESP32-S3-BOX-3
- **ESP-IDF**：v6.0.1
- **屏幕**：320×240 LCD，电容触摸

## 快速开始

### 1. 配置 ESP-IDF 环境

将 `idf-env.example.ps1` 复制为 `idf-env.ps1`（如需 CMD 支持，同时把
`idf-env.example.bat` 复制为 `idf-env.bat`），填入本机的 ESP-IDF 路径。
`idf-env.ps1`/`idf-env.bat` 均已加入 `.gitignore`，属于本机配置，不会被提交。

### 2. 配置 WiFi

```powershell
. .\build_esp32.ps1 -ActivateOnly
idf.py menuconfig
```

进入 **"LVGL UI Template Configuration" → "Wi-Fi"**，设置 `APP_WIFI_SSID` /
`APP_WIFI_PASSWORD`。未配置 WiFi 时，截图/输入调试服务不会启动（它们需要设备
获得 IP 地址）。

### 3. 编译、烧录、监控

```powershell
powershell -NoProfile -Command ". '$PWD\build_esp32.ps1' -ActivateOnly; idf.py -p COM3 flash monitor"
```

> **仅首次编译需注意**：解析 `espressif/esp-box-3` 组件依赖需要访问 ESP
> 组件仓库（Component Registry）。如果你的网络需要代理，请在运行 `idf.py`
> 前设置 `HTTP_PROXY` / `HTTPS_PROXY` 环境变量，详见 [CLAUDE.md](CLAUDE.md)。
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

完整的坐标系参考和推荐的 agent 工作流，见 [CLAUDE.md](CLAUDE.md)。

## 项目结构

```
lvgl-ui-template/
├── CMakeLists.txt              # 根 CMake，通过 APP_BOARD 选择 board
├── sdkconfig.defaults          # PSRAM + LVGL snapshot + 调试服务
├── partitions.csv
├── main/                        # app_main：初始化 HAL、UI、WiFi、调试服务
├── components/
│   ├── board_hal/               # 通用 display/touch/storage 接口
│   ├── boards/esp32s3_box3/      # board 相关 HAL 实现
│   ├── ui/                       # ui_theme、ui_manager、4 个示例界面
│   ├── wifi_manager/
│   ├── debug_screenshot/         # TCP 3333
│   └── debug_input/               # TCP 3334
└── tools/                        # PC 端 Python 工具
```

## License

MIT
