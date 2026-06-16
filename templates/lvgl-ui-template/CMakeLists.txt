cmake_minimum_required(VERSION 3.24)

# ── Board selection ────────────────────────────────────────────────────────────
#
# Priority (highest → lowest):
#   1. -DAPP_BOARD=<name>  on the cmake/idf.py command line
#   2. CONFIG_APP_BOARD_* parsed from sdkconfig (updated immediately by menuconfig)
#   3. Default: esp32s3_box3
#
# We read sdkconfig directly (not sdkconfig.cmake) because sdkconfig.cmake is
# generated during the build phase — reading it here would be one build cycle
# behind after a menuconfig change, causing the wrong board component to link.

if(NOT DEFINED APP_BOARD)
    set(APP_BOARD "esp32s3_box3")   # default

    set(_sdkconfig "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig")
    if(EXISTS "${_sdkconfig}")
        file(STRINGS "${_sdkconfig}" _sdk_lines REGEX "^CONFIG_APP_BOARD_")
        foreach(_line IN LISTS _sdk_lines)
            if(_line MATCHES "^CONFIG_APP_BOARD_ESP32S3_LCD_EV_BOARD=y")
                set(APP_BOARD "esp32s3_lcd_ev_board")
            elseif(_line MATCHES "^CONFIG_APP_BOARD_CUSTOM=y")
                set(APP_BOARD "__custom__")
            endif()
        endforeach()

        if(APP_BOARD STREQUAL "__custom__")
            file(STRINGS "${_sdkconfig}" _name_lines
                 REGEX "^CONFIG_APP_BOARD_CUSTOM_NAME=")
            foreach(_line IN LISTS _name_lines)
                string(REGEX REPLACE
                    "^CONFIG_APP_BOARD_CUSTOM_NAME=\"(.+)\"$" "\\1"
                    _custom_name "${_line}")
                set(APP_BOARD "${_custom_name}")
            endforeach()
            if(APP_BOARD STREQUAL "__custom__")
                set(APP_BOARD "custom")   # safety fallback
            endif()
        endif()
    endif()
endif()

message(STATUS "APP_BOARD = ${APP_BOARD}")

# ── sdkconfig defaults (stacked, last file wins) ───────────────────────────────
set(SDKCONFIG_DEFAULTS
    "sdkconfig.defaults"
    "sdkconfig.defaults.esp32s3"
)

# Board-specific overrides (only applied on fresh sdkconfig creation)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults.${APP_BOARD}")
    list(APPEND SDKCONFIG_DEFAULTS "sdkconfig.defaults.${APP_BOARD}")
endif()

# ── Component paths ────────────────────────────────────────────────────────────
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/components"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/boards/${APP_BOARD}"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lvgl_ui_template)

# ── Upstream managed-component workaround ─────────────────────────────────────
# espressif__esp_lcd_gc9503 uses a pointer after free() inside an ESP_LOGD macro
# (the pointer value is only read for its address in the log string, but GCC 13+
# flags it as -Wuse-after-free).  We cannot modify managed component sources, so
# suppress the diagnostic globally — it is harmless false-positive in this case.
idf_build_set_property(COMPILE_OPTIONS "-Wno-use-after-free" APPEND)
