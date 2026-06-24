#include "esp_log.h"
#include "lvgl.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

#define TAG "UI_PAGE3"

static void back_cb(lv_event_t *e) { (void)e; ui_manager_pop(UI_ANIM_SLIDE_RIGHT); }

static void list_item_cb(lv_event_t *e)
{
    lv_obj_t *list  = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *btn   = lv_event_get_target(e);
    const char *txt = lv_list_get_btn_text(list, btn);
    ESP_LOGI(TAG, "list item selected: %s", txt ? txt : "?");
}

lv_obj_t *screen_page3_create(void)
{
    const ui_palette_t *p = ui_theme_palette();
    lv_obj_t *scr = lv_obj_create(NULL);
    ui_theme_style_root(scr);

    lv_obj_t *bar = ui_theme_create_title_bar(scr, "Page 3: List");
    ui_theme_create_back_button(bar, back_cb);

    lv_obj_t *list = lv_list_create(scr);
    lv_obj_set_size(list, LV_PCT(75), LV_PCT(78));
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 18);
    lv_obj_set_style_bg_color(list, p->panel, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(list, p->border, 0);
    lv_obj_set_style_border_width(list, 1, 0);
    lv_obj_set_style_pad_row(list, 6, 0);
    lv_obj_set_style_pad_all(list, 8, 0);
    lv_obj_set_style_radius(list, 10, 0);

    const char *items[] = {"Item One", "Item Two", "Item Three", "Item Four", "Item Five"};
    for (int i = 0; i < (int)(sizeof(items) / sizeof(items[0])); i++) {
        lv_obj_t *btn = lv_list_add_btn(list, LV_SYMBOL_FILE, items[i]);
        ui_theme_style_panel_alt(btn);
        lv_obj_set_style_text_color(btn, p->text, 0);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_16, 0);
        lv_obj_add_event_cb(btn, list_item_cb, LV_EVENT_CLICKED, list);
    }

    return scr;
}
