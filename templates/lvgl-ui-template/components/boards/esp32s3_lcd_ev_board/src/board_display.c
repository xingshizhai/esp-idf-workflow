#include "esp_log.h"
#include "esp_check.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "esp_lvgl_port.h"
#include "esp_lvgl_port_disp.h"
#include "board_hal/hal_display.h"
#include "board_hal/hal.h"

#define TAG "DISP"

static hal_display_t s_display;
static lv_display_t *s_lv_disp = NULL;

/* Called by board_touch.c after touch is ready */
lv_display_t *board_display_get_lv_disp(void)
{
    return s_lv_disp;
}

esp_err_t hal_display_create(const hal_display_cfg_t *cfg, hal_display_t **out)
{
    /* Init LVGL port (task + timer) */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "lvgl_port_init failed");

    /* Create RGB LCD panel.
     * BSP allocates CONFIG_BSP_LCD_RGB_BUFFER_NUMS frame buffers in PSRAM.
     * Must be 2 to support avoid_tearing mode (see sdkconfig.defaults). */
    esp_lcd_panel_handle_t    panel_handle = NULL;
    esp_lcd_panel_io_handle_t io_handle    = NULL;
    bsp_display_config_t bsp_disp_cfg = { .max_transfer_sz = 0 };
    ESP_RETURN_ON_ERROR(bsp_display_new(&bsp_disp_cfg, &panel_handle, &io_handle),
                        TAG, "bsp_display_new failed");

    /* Resolution is runtime on this BSP (sub-board selected via Kconfig) */
    uint16_t h_res = bsp_display_get_h_res();
    uint16_t v_res = bsp_display_get_v_res();

    /* Bounce buffer mode: the panel DMA reads from a small SRAM bounce buffer
     * instead of directly from the PSRAM frame buffer.  A background task
     * refills the SRAM buffer from PSRAM.  This eliminates the PSRAM bandwidth
     * contention between the CPU and LCD DMA that causes the horizontal scroll
     * artifact.  LVGL allocates its own draw buffer in SPIRAM and writes to
     * the panel frame buffer via draw_bitmap; since the DMA never reads PSRAM
     * directly there is no conflict. */
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle    = io_handle,
        .panel_handle = panel_handle,
        .buffer_size  = h_res * 40,   /* 40-line partial draw buffer in SPIRAM */
        .double_buffer = true,
        .hres         = h_res,
        .vres         = v_res,
        .monochrome   = false,
        .rotation = {
            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_spiram  = true,
            .swap_bytes   = false,
        },
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode       = true,   /* sync to on_bounce_frame_finish callback */
            .avoid_tearing = false,
        }
    };
    s_lv_disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);
    if (!s_lv_disp) {
        ESP_LOGE(TAG, "lvgl_port_add_disp_rgb failed");
        return ESP_FAIL;
    }

    s_display.backlight_set = NULL;
    *out = &s_display;
    ESP_LOGI(TAG, "display ready %dx%d (RGB, bounce_buffer mode)", h_res, v_res);
    return ESP_OK;
}

void hal_display_destroy(hal_display_t *disp) { }
