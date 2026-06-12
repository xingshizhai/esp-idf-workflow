idf_component_register(
    SRCS
        "src/board_display.c"
        "src/board_touch.c"
    INCLUDE_DIRS "include"
    REQUIRES
        board_hal
        log
    PRIV_REQUIRES
        espressif__esp_lvgl_port
        espressif__esp-box-3
        espressif__esp_lcd_touch
)
