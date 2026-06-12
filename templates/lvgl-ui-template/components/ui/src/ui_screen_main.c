#include "lvgl.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

static void btn_page1_cb(lv_event_t *e)
{
    (void)e;
    ui_manager_push(UI_SCREEN_PAGE1, UI_ANIM_SLIDE_LEFT);
}

static void btn_page2_cb(lv_event_t *e)
{
    (void)e;
    ui_manager_push(UI_SCREEN_PAGE2, UI_ANIM_SLIDE_LEFT);
}

static void btn_page3_cb(lv_event_t *e)
{
    (void)e;
    ui_manager_push(UI_SCREEN_PAGE3, UI_ANIM_SLIDE_LEFT);
}

static lv_obj_t *create_nav_button(lv_obj_t *parent, const char *label, int16_t y, lv_event_cb_t cb)
{
    const ui_palette_t *p = ui_theme_palette();

    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_set_size(btn, 280, 52);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y);
    ui_theme_style_button_primary(btn);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_set_style_text_color(lbl, p->text, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    lv_obj_center(lbl);

    return btn;
}

lv_obj_t *screen_main_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    ui_theme_style_root(scr);

    /* ── Title bar ────────────────────────────────────────────────── */
    ui_theme_create_title_bar(scr, "LVGL UI Template");

    /* ── Three navigation buttons → three demo pages ─────────────────── */
    create_nav_button(scr, "Page 1: Labels",        50, btn_page1_cb);
    create_nav_button(scr, "Page 2: Slider",       112, btn_page2_cb);
    create_nav_button(scr, "Page 3: List",         174, btn_page3_cb);

    return scr;
}
