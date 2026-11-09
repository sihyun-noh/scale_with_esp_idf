#include <string.h>

#include "lvgl.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "eez_ui_port.h"
#include "eez_ui_binding.h"
#include "eez_ui_flow_lifecycle.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/ui.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/eez-flow.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/screens.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/fonts.h"

/* EEZ exports this C API from generated eez-flow.cpp. */
extern bool eez_flow_is_stopped(void);

static const char *TAG = "EezUiPort7B";
static lv_timer_t *s_eez_tick_timer;
static lv_obj_t *s_upper_product_list;
static lv_obj_t *s_lower_product_list;
static bool s_startup_guard_active;
static int64_t s_startup_started_at_us;
static int16_t s_last_logged_screen = -1;

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
    int16_t screen = eez_flow_get_current_screen();
    if (s_startup_guard_active) {
        const int64_t elapsed_us = esp_timer_get_time() - s_startup_started_at_us;
        if (screen == SCREEN_ID_DASHBOARD && elapsed_us >= 1800000) {
            s_startup_guard_active = false;
        } else if (objects.startup_animation != NULL &&
                   (screen != SCREEN_ID_STARTUP_ANIMATION ||
                    lv_screen_active() != objects.startup_animation)) {
            /* The startup flow has only one intended transition: Dashboard.
             * Keep incidental Work/input page loads from replacing the splash. */
            ESP_LOGW(TAG, "unexpected startup screen %d at %lld ms; restoring splash",
                     (int)screen, (long long)(elapsed_us / 1000));
            eez_flow_set_screen(SCREEN_ID_STARTUP_ANIMATION, (lv_scr_load_anim_t)0, 0, 0);
            screen = SCREEN_ID_STARTUP_ANIMATION;
        }
    }
    if (screen != s_last_logged_screen) {
        ESP_LOGI(TAG, "EEZ screen: id=%d", (int)screen);
        s_last_logged_screen = screen;
    }
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

    ESP_LOGI(TAG, "app entry: screen=%d startup_active=%d",
             (int)eez_flow_get_current_screen(),
             lv_screen_active() == objects.startup_animation);
    s_startup_guard_active = true;
    s_startup_started_at_us = esp_timer_get_time();
    s_last_logged_screen = -1;

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
    s_startup_guard_active = false;
    s_startup_started_at_us = 0;
    s_last_logged_screen = -1;
    ESP_LOGI(TAG, "EEZ UI stopped");
}
