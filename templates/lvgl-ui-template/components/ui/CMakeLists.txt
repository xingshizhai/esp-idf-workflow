idf_component_register(
    SRCS
        "src/ui_theme.c"
        "src/ui_manager.c"
        "src/ui_screen_main.c"
        "src/ui_screen_page1.c"
        "src/ui_screen_page2.c"
        "src/ui_screen_page3.c"
    INCLUDE_DIRS "include"
    REQUIRES
        log
        lvgl__lvgl
    PRIV_REQUIRES
        espressif__esp_lvgl_port
)
