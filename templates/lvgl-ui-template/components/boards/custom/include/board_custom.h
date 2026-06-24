#pragma once

#include "esp_err.h"

/* ── Board identity ───────────────────────────────────────────────────────────
 * Change BOARD_NAME and the LCD dimensions to match your hardware.          */

#define BOARD_NAME       "Custom Board"
#define BOARD_LCD_WIDTH  320
#define BOARD_LCD_HEIGHT 240

esp_err_t board_init_all(void);
