#include "lvgl.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

static void back_cb(lv_event_t *e) { (void)e; ui_manager_pop(UI_ANIM_SLIDE_RIGHT); }

lv_obj_t *screen_page1_create(void)
{
    const ui_palette_t *p = ui_theme_palette();
    lv_obj_t *scr = lv_obj_create(NULL);
    ui_theme_style_root(scr);

    lv_obj_t *bar = ui_theme_create_title_bar(scr, "Page 1: Labels");
    ui_theme_create_back_button(bar, back_cb);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, LV_PCT(88), LV_SIZE_CONTENT);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 18);
    ui_theme_style_panel(card);
    lv_obj_set_style_pad_all(card, 20, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Static text demo");
    lv_obj_set_style_text_color(title, p->accent, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *body = lv_label_create(card);
    lv_label_set_text(body,
        "This screen shows a simple label layout.\n\n"
        "Use it as a starting point for status text, sensor readings,\n"
        "or any read-only content that needs to display on screen.");
    lv_obj_set_style_text_color(body, p->text_muted, 0);
    lv_obj_set_style_text_font(body, &lv_font_montserrat_16, 0);
    lv_obj_set_width(body, LV_PCT(100));
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_pad_top(body, 16, 0);

    return scr;
}
