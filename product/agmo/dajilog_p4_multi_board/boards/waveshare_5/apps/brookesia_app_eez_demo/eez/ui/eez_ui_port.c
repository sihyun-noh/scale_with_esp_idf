#include <string.h>

#include "lvgl.h"
#include "esp_log.h"
#include "eez_ui_port.h"
#include "eez_ui_binding.h"
#include "eez_ui_flow_lifecycle.h"
#include "generated/brookesia_dajilog/src/ui/ui.h"
#include "generated/brookesia_dajilog/src/ui/screens.h"

/* EEZ exports these C APIs from generated eez-flow.cpp. */
extern int16_t eez_flow_get_current_screen(void);
extern bool eez_flow_is_stopped(void);
extern void eez_flow_set_screen(int16_t screen_id, lv_scr_load_anim_t anim_type,
                                uint32_t speed, uint32_t delay);

#define STARTUP_FALLBACK_TIMEOUT_MS 3000U

static const char *TAG = "EezUiPort";
static lv_timer_t *s_eez_tick_timer;
static uint32_t s_startup_tick;
static int16_t s_last_screen;
static bool s_startup_fallback_done;

static void eez_ui_tick_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    ui_tick();
    eez_ui_binding_process();

    const int16_t current_screen = eez_flow_get_current_screen();
    if (current_screen != s_last_screen) {
        ESP_LOGI(TAG, "screen changed: %d -> %d", s_last_screen, current_screen);
        s_last_screen = current_screen;
    }

    if (!s_startup_fallback_done &&
        current_screen == SCREEN_ID_STARTUP_ANIMATION &&
        lv_tick_elaps(s_startup_tick) >= STARTUP_FALLBACK_TIMEOUT_MS) {
        s_startup_fallback_done = true;
        ESP_LOGW(TAG, "startup Flow timeout; forcing Main screen");
        eez_flow_set_screen(SCREEN_ID_MAIN, LV_SCR_LOAD_ANIM_FADE_IN, 250, 0);
    }
}

bool eez_ui_port_create(void)
{
    /* A previous app instance must have completed Flow shutdown. */
    if (!eez_flow_is_stopped()) {
        ESP_LOGW(TAG, "stale EEZ Flow detected; stopping before restart");
        eez_ui_flow_stop();
    }

    ui_init();

    if (objects.main == NULL) {
        return false;
    }

    if (!eez_ui_binding_attach()) {
        return false;
    }

    s_startup_tick = lv_tick_get();
    s_last_screen = eez_flow_get_current_screen();
    s_startup_fallback_done = false;
    ESP_LOGI(TAG, "initial screen: %d", s_last_screen);

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
        /* Brookesia owns and deletes the recorded timer after this callback. */
        lv_timer_pause(s_eez_tick_timer);
    }

    eez_ui_binding_detach();
    eez_ui_flow_stop();
    /* Brookesia's resource recycler deletes objects and timers created in run(). */
    s_eez_tick_timer = NULL;
    s_startup_tick = 0;
    s_last_screen = 0;
    s_startup_fallback_done = false;
    memset(&objects, 0, sizeof(objects));
    ESP_LOGI(TAG, "EEZ UI stopped");
}
