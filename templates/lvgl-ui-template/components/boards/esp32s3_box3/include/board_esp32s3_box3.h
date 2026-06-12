#pragma once

#define BOARD_NAME          "ESP32-S3-BOX-3"
#define BOARD_LCD_WIDTH     320
#define BOARD_LCD_HEIGHT    240

#include "esp_err.h"

esp_err_t board_init_all(void);
