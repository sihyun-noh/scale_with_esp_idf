#include <string.h>

#include "lvgl.h"
#include "esp_log.h"
#include "eez_ui_port.h"
#include "eez_ui_binding.h"
#include "eez_ui_flow_lifecycle.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/ui.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/screens.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/fonts.h"

/* EEZ exports this C API from generated eez-flow.cpp. */
extern bool eez_flow_is_stopped(void);

static const char *TAG = "EezUiPort7B";
static lv_timer_t *s_eez_tick_timer;
static lv_obj_t *s_upper_product_list;
static lv_obj_t *s_lower_product_list;

static void style_product_dropdown_list(lv_obj_t *dropdown, lv_obj_t **styled_list)
{
    lv_obj_t *list = dropdown != NULL ? lv_dropdown_get_list(dropdown) : NULL;
    if (list == NULL) {
        *styled_list = NULL;
        return;
    }
    if (list == *styled_list) {
        return;
    }

    /* LVGL creates the popup list separately on the top layer. It does not
     * inherit the Korean font configured for the closed EEZ dropdown. */
    lv_obj_set_style_text_font(list, &ui_font_maru_buri_regular_kr20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(list, lv_color_hex(0xeef5fa),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x16212b),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    *styled_list = list;
}

static void eez_ui_tick_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    ui_tick();
    style_product_dropdown_list(objects.main_upper_product, &s_upper_product_list);
    style_product_dropdown_list(objects.main_lower_product, &s_lower_product_list);
    eez_ui_binding_process();
}

bool eez_ui_port_create(void)
{
    if (!eez_flow_is_stopped()) {
        ESP_LOGW(TAG, "stale EEZ Flow detected; stopping before restart");
        eez_ui_flow_stop();
    }

    ui_init();

    if (!eez_ui_binding_attach()) {
        return false;
    }

    s_eez_tick_timer = lv_timer_create(eez_ui_tick_timer_cb, 10, NULL);
    if (s_eez_tick_timer == NULL) {
        eez_ui_binding_detach();
        return false;
    }

    return true;
}

void eez_ui_port_prepare_cleanup(void)
{
    if (s_eez_tick_timer != NULL) {
        lv_timer_pause(s_eez_tick_timer);
    }

    eez_ui_binding_detach();
    eez_ui_flow_stop();
    s_eez_tick_timer = NULL;
    memset(&objects, 0, sizeof(objects));
    s_upper_product_list = NULL;
    s_lower_product_list = NULL;
    ESP_LOGI(TAG, "EEZ UI stopped");
}
