#include <string.h>

#include "lvgl.h"
#include "esp_log.h"
#include "eez_ui_port.h"
#include "eez_ui_binding.h"
#include "eez_ui_flow_lifecycle.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/ui.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/screens.h"

/* EEZ exports this C API from generated eez-flow.cpp. */
extern bool eez_flow_is_stopped(void);

static const char *TAG = "EezUiPort7B";
static lv_timer_t *s_eez_tick_timer;

static void eez_ui_tick_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    ui_tick();
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
    ESP_LOGI(TAG, "EEZ UI stopped");
}
