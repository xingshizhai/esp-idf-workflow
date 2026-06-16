/* board_display.c — custom board skeleton
 *
 * Copy this directory to components/boards/<your_name>/ and fill in the
 * TODOs below.  The only contract with the rest of the firmware is:
 *
 *   esp_err_t hal_display_create(const hal_display_cfg_t *cfg,
 *                                hal_display_t **out);
 *   void      hal_display_destroy(hal_display_t *disp);
 *
 * and that board_display_get_lv_disp() returns the registered lv_display_t*
 * so that board_touch.c can pass it to lvgl_port_add_touch().
 */

#include "esp_log.h"
#include "esp_check.h"
#include "esp_lvgl_port.h"
#include "esp_lvgl_port_disp.h"
#include "board_hal/hal_display.h"
#include "board_hal/hal.h"

/* TODO: include your LCD panel driver header, e.g.:
 *   #include "esp_lcd_ili9341.h"
 *   #include "driver/spi_master.h"
 */

#define TAG "DISP"

static hal_display_t s_display;
static lv_display_t *s_lv_disp = NULL;

lv_display_t *board_display_get_lv_disp(void)
{
    return s_lv_disp;
}

/* Optional: implement backlight control via PWM/GPIO.
 * Set s_display.backlight_set = NULL if not supported. */
static esp_err_t display_backlight_set(hal_display_t *disp, uint8_t percent)
{
    /* TODO: drive backlight PWM / GPIO */
    (void)disp; (void)percent;
    return ESP_OK;
}

esp_err_t hal_display_create(const hal_display_cfg_t *cfg, hal_display_t **out)
{
    /* 1. Init LVGL port */
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "lvgl_port_init failed");

    /* 2. TODO: initialise your SPI/I2C/RGB LCD bus and panel.
     *    Example for a SPI panel (ILI9341 / ST7789 etc.):
     *
     *    spi_bus_config_t bus_cfg = { .mosi_io_num = YOUR_MOSI, ... };
     *    spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
     *
     *    esp_lcd_panel_io_spi_config_t io_cfg = { .cs_gpio_num = YOUR_CS, ... };
     *    esp_lcd_new_panel_io_spi(SPI2_HOST, &io_cfg, &io_handle);
     *
     *    esp_lcd_panel_dev_config_t panel_cfg = { .bits_per_pixel = 16, ... };
     *    esp_lcd_new_panel_ili9341(io_handle, &panel_cfg, &panel_handle);
     *
     *    esp_lcd_panel_reset(panel_handle);
     *    esp_lcd_panel_init(panel_handle);
     *    esp_lcd_panel_disp_on_off(panel_handle, true);
     */
    esp_lcd_panel_handle_t    panel_handle = NULL;
    esp_lcd_panel_io_handle_t io_handle    = NULL;
    /* TODO: populate panel_handle and io_handle */

    /* 3. Register with LVGL.
     *    For SPI/I2C panels use lvgl_port_add_disp().
     *    For RGB panels use lvgl_port_add_disp_rgb() — set io_handle = NULL. */
    uint32_t h_res   = cfg ? cfg->width  : BOARD_LCD_WIDTH;
    uint32_t v_res   = cfg ? cfg->height : BOARD_LCD_HEIGHT;
    uint32_t buf_px  = (cfg && cfg->buf_size_px) ? cfg->buf_size_px : h_res * 50;
    bool double_buf  = cfg ? cfg->double_buffered : false;

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle     = io_handle,
        .panel_handle  = panel_handle,
        .buffer_size   = buf_px,
        .double_buffer = double_buf,
        .hres          = h_res,
        .vres          = v_res,
        .monochrome    = false,
        .rotation = {
            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma    = true,   /* set false and buff_spiram=true for RGB/PSRAM */
            .buff_spiram = false,
            .swap_bytes  = false,
        },
    };
    s_lv_disp = lvgl_port_add_disp(&disp_cfg);
    if (!s_lv_disp) {
        ESP_LOGE(TAG, "lvgl_port_add_disp failed");
        return ESP_FAIL;
    }

    s_display.backlight_set = display_backlight_set; /* or NULL if unsupported */
    *out = &s_display;
    ESP_LOGI(TAG, "display ready %ldx%ld", h_res, v_res);
    return ESP_OK;
}

void hal_display_destroy(hal_display_t *disp) { }
