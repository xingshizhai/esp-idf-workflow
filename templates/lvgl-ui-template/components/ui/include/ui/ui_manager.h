#pragma once
#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    UI_SCREEN_MAIN  = 0,
    UI_SCREEN_PAGE1,
    UI_SCREEN_PAGE2,
    UI_SCREEN_PAGE3,
    UI_SCREEN_MAX,
} ui_screen_id_t;

typedef enum {
    UI_ANIM_NONE       = 0,
    UI_ANIM_FADE,
    UI_ANIM_SLIDE_LEFT,
    UI_ANIM_SLIDE_RIGHT,
} ui_anim_type_t;

esp_err_t ui_manager_init(void);
void      ui_manager_deinit(void);

esp_err_t      ui_manager_show(ui_screen_id_t id, ui_anim_type_t anim);
esp_err_t      ui_manager_push(ui_screen_id_t id, ui_anim_type_t anim);
esp_err_t      ui_manager_pop(ui_anim_type_t anim);
ui_screen_id_t ui_manager_current(void);
