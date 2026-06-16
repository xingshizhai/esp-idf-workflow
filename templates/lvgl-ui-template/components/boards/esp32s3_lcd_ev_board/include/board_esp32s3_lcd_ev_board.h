#pragma once

#include "sdkconfig.h"
#include "esp_err.h"

#define BOARD_NAME "ESP32-S3-LCD-EV-BOARD-2"

#if CONFIG_BSP_LCD_SUB_BOARD_480_480
#define BOARD_LCD_WIDTH  480
#define BOARD_LCD_HEIGHT 480
#elif CONFIG_BSP_LCD_SUB_BOARD_800_480
#define BOARD_LCD_WIDTH  800
#define BOARD_LCD_HEIGHT 480
#else
/* No sub-board selected — defaults to 480x480 */
#define BOARD_LCD_WIDTH  480
#define BOARD_LCD_HEIGHT 480
#endif

esp_err_t board_init_all(void);
