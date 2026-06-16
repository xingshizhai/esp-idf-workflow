#include "lvgl.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

static void btn_page1_cb(lv_event_t *e) { (void)e; ui_manager_push(UI_SCREEN_PAGE1, UI_ANIM_SLIDE_LEFT); }
static void btn_page2_cb(lv_event_t *e) { (void)e; ui_manager_push(UI_SCREEN_PAGE2, UI_ANIM_SLIDE_LEFT); }
static void btn_page3_cb(lv_event_t *e) { (void)e; ui_manager_push(UI_SCREEN_PAGE3, UI_ANIM_SLIDE_LEFT); }

lv_obj_t *screen_main_create(void)
{
    const ui_palette_t *p = ui_theme_palette();
    lv_obj_t *scr = lv_obj_create(NULL);
    ui_theme_style_root(scr);

    ui_theme_create_title_bar(scr, "LVGL UI Template");

    /* Flex container centred vertically in the content area below the title bar */
    lv_obj_t *col = lv_obj_create(scr);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_style_pad_row(col, 20, 0);
    lv_obj_set_size(col, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(col, LV_ALIGN_CENTER, 0, 18);   /* +18: offset for title bar */
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

    const char *labels[] = {"Page 1: Labels", "Page 2: Slider", "Page 3: List"};
    lv_event_cb_t cbs[]  = {btn_page1_cb, btn_page2_cb, btn_page3_cb};

    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_obj_create(col);
        lv_obj_set_size(btn, LV_PCT(75), 64);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        ui_theme_style_button_primary(btn);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, cbs[i], LV_EVENT_CLICKED, NULL);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_color(lbl, p->text, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
        lv_obj_center(lbl);
    }

    return scr;
}
