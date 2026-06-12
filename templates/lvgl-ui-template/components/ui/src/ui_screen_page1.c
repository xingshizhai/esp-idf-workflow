#include "lvgl.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

static void back_cb(lv_event_t *e)
{
    (void)e;
    ui_manager_pop(UI_ANIM_SLIDE_RIGHT);
}

lv_obj_t *screen_page1_create(void)
{
    const ui_palette_t *p = ui_theme_palette();
    lv_obj_t *scr = lv_obj_create(NULL);
    ui_theme_style_root(scr);

    lv_obj_t *bar = ui_theme_create_title_bar(scr, "Page 1: Labels");
    ui_theme_create_back_button(bar, back_cb);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, 296, 180);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 44);
    ui_theme_style_panel(card);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Static text demo");
    lv_obj_set_style_text_color(title, p->accent, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *body = lv_label_create(card);
    lv_label_set_text(body,
        "This screen shows a simple label layout.\n\n"
        "Use it as a starting point for status\n"
        "text, sensor readings, or any read-only\n"
        "content.");
    lv_obj_set_style_text_color(body, p->text_muted, 0);
    lv_obj_set_style_text_font(body, &lv_font_montserrat_14, 0);
    lv_obj_set_width(body, 280);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 0, 32);

    return scr;
}
