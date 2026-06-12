#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

#define SCREEN_STACK_DEPTH 4

static ui_screen_id_t s_stack[SCREEN_STACK_DEPTH];
static int            s_stack_top = -1;
static lv_obj_t      *s_screens[UI_SCREEN_MAX] = {0};

/* Defined in the per-screen source files */
lv_obj_t *screen_main_create(void);
lv_obj_t *screen_page1_create(void);
lv_obj_t *screen_page2_create(void);
lv_obj_t *screen_page3_create(void);

static lv_obj_t *create_screen(ui_screen_id_t id)
{
    switch (id) {
    case UI_SCREEN_MAIN:  return screen_main_create();
    case UI_SCREEN_PAGE1: return screen_page1_create();
    case UI_SCREEN_PAGE2: return screen_page2_create();
    case UI_SCREEN_PAGE3: return screen_page3_create();
    default:              return NULL;
    }
}

/* UI_ANIM_NONE uses lv_scr_load() directly so that lv_scr_load_anim()'s
 * animation infrastructure (which allocates a full-screen ARGB layer buffer
 * in PSRAM) is never triggered. On this hardware the soft renderer takes
 * ~150 ms per frame, so animated transitions can freeze mid-way on complex
 * screens — prefer UI_ANIM_NONE there. */
static void do_scr_load(lv_obj_t *scr, ui_anim_type_t anim)
{
    if (anim == UI_ANIM_NONE) {
        lv_scr_load(scr);
    } else {
        lv_scr_load_anim(scr,
                          anim == UI_ANIM_FADE       ? LV_SCR_LOAD_ANIM_FADE_ON    :
                          anim == UI_ANIM_SLIDE_LEFT  ? LV_SCR_LOAD_ANIM_MOVE_LEFT  :
                                                        LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                          200, 0, false);
    }
}

esp_err_t ui_manager_init(void)
{
    /* All LVGL object creation must happen inside the port lock because the
     * LVGL task is already running by the time we get here. */
    if (!lvgl_port_lock(0)) return ESP_FAIL;

    for (int i = 0; i < UI_SCREEN_MAX; i++) {
        s_screens[i] = create_screen((ui_screen_id_t)i);
    }

    lvgl_port_unlock();
    return ESP_OK;
}

void ui_manager_deinit(void)
{
    for (int i = 0; i < UI_SCREEN_MAX; i++) {
        if (s_screens[i]) lv_obj_delete(s_screens[i]);
    }
}

esp_err_t ui_manager_show(ui_screen_id_t id, ui_anim_type_t anim)
{
    if (id >= UI_SCREEN_MAX) return ESP_ERR_INVALID_ARG;
    if (lvgl_port_lock(100)) {
        do_scr_load(s_screens[id], anim);
        s_stack_top = 0;
        s_stack[0]  = id;
        lvgl_port_unlock();
    }
    return ESP_OK;
}

esp_err_t ui_manager_push(ui_screen_id_t id, ui_anim_type_t anim)
{
    if (id >= UI_SCREEN_MAX) return ESP_ERR_INVALID_ARG;
    if (s_stack_top < SCREEN_STACK_DEPTH - 1) {
        s_stack[++s_stack_top] = id;
    }
    if (lvgl_port_lock(100)) {
        do_scr_load(s_screens[id], anim);
        lvgl_port_unlock();
    }
    return ESP_OK;
}

esp_err_t ui_manager_pop(ui_anim_type_t anim)
{
    if (s_stack_top > 0) s_stack_top--;
    ui_screen_id_t next = s_stack[s_stack_top];
    if (lvgl_port_lock(100)) {
        do_scr_load(s_screens[next], anim);
        lvgl_port_unlock();
    }
    return ESP_OK;
}

ui_screen_id_t ui_manager_current(void)
{
    return s_stack_top >= 0 ? s_stack[s_stack_top] : UI_SCREEN_MAIN;
}
