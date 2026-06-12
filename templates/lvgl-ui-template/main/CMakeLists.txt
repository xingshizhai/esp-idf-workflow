idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        board_hal
        ui
        wifi_manager
        debug_screenshot
        debug_input
        nvs_flash
        esp_event
        esp_netif
        esp_timer
        log
        freertos
)
