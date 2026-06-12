idf_component_register(
    SRCS "src/debug_input.c"
    INCLUDE_DIRS "include"
    REQUIRES
        lvgl__lvgl
        espressif__esp_lvgl_port
        freertos
        log
        lwip
)
