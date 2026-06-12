#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "board_hal/hal.h"
#include "ui/ui_manager.h"
#include "wifi_manager.h"
#include "debug_screenshot.h"
#include "debug_input.h"

#define TAG "MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "lvgl-ui-template starting");

    /* 1. Default event loop + TCP/IP stack (required before Wi-Fi) */
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();

    /* 2. Storage (NVS) — must be first; Wi-Fi requires NVS to be initialized */
    ESP_ERROR_CHECK(hal_storage_init());

    /* 3. Display + touch */
    hal_display_cfg_t disp_cfg = {
        .width           = 320,
        .height          = 240,
        .rotation        = 0,
        .double_buffered = true,
        .buf_size_px     = 320 * 50,
    };
    ESP_ERROR_CHECK(hal_display_create(&disp_cfg, &g_hal.display));
    ESP_ERROR_CHECK(hal_touch_create(&g_hal.touch));

    /* 4. UI */
    ESP_ERROR_CHECK(ui_manager_init());
    ui_manager_show(UI_SCREEN_MAIN, UI_ANIM_NONE);

    /* 5. Wi-Fi + debug servers — start after the display/LVGL buffers
     * (internal RAM) are allocated, since the Wi-Fi driver's RX/TX buffers
     * also compete for internal RAM. */
    ESP_ERROR_CHECK(wifi_manager_start());
    ESP_ERROR_CHECK(debug_screenshot_start());
    ESP_ERROR_CHECK(debug_input_start());

    ESP_LOGI(TAG, "startup complete");
}
