#include <stdio.h>
#include "lvgl.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

static lv_obj_t *s_value_label = NULL;

static void back_cb(lv_event_t *e) { (void)e; ui_manager_pop(UI_ANIM_SLIDE_RIGHT); }

static void slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    if (s_value_label) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%ld%%", (long)val);
        lv_label_set_text(s_value_label, buf);
    }
}

lv_obj_t *screen_page2_create(void)
{
    const ui_palette_t *p = ui_theme_palette();
    lv_obj_t *scr = lv_obj_create(NULL);
    ui_theme_style_root(scr);

    lv_obj_t *bar = ui_theme_create_title_bar(scr, "Page 2: Slider");
    ui_theme_create_back_button(bar, back_cb);

    lv_obj_t *card = lv_obj_create(scr);
    lv_obj_set_size(card, LV_PCT(75), LV_SIZE_CONTENT);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 18);
    ui_theme_style_panel(card);
    lv_obj_set_style_pad_all(card, 24, 0);
    lv_obj_set_style_pad_row(card, 16, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *row = lv_obj_create(card);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, "Value");
    lv_obj_set_style_text_color(lbl, p->text_muted, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    s_value_label = lv_label_create(row);
    lv_label_set_text(s_value_label, "50%");
    lv_obj_set_style_text_color(s_value_label, p->accent, 0);
    lv_obj_set_style_text_font(s_value_label, &lv_font_montserrat_24, 0);
    lv_obj_align(s_value_label, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t *slider = lv_slider_create(card);
    lv_obj_set_size(slider, LV_PCT(100), 16);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, p->panel_alt, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, p->accent,    LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, p->accent,    LV_PART_KNOB);
    lv_obj_add_event_cb(slider, slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    return scr;
}
