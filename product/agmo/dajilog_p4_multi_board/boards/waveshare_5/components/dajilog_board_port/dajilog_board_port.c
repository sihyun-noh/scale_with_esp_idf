#include "dajilog_board.h"

#include "bsp/esp-bsp.h"
#include "esp_lv_adapter.h"

static const dajilog_board_info_t s_board_info = {
    .board_name = "Waveshare ESP32-P4-WIFI6-Touch-LCD-5",
    .display_width = 720,
    .display_height = 1280,
    .ui_profile = DAJILOG_UI_PROFILE_PORTRAIT_720_1280,
    .has_touch = true,
    .has_wifi = true,
    .has_usb_host = true,
    .has_can = false,
    .has_rs485 = false,
    .has_sdi12 = false,
};

static const dajilog_board_can_config_t s_can_config = {
    .supported = false,
    .tx_gpio = -1,
    .rx_gpio = -1,
    .transceiver_enable_gpio = -1,
    .bitrate = 500000U,
};

const dajilog_board_info_t *dajilog_board_get_info(void)
{
    return &s_board_info;
}

const dajilog_board_can_config_t *dajilog_board_get_can_config(void)
{
    return &s_can_config;
}

esp_err_t dajilog_board_init(void)
{
    return ESP_OK;
}

esp_err_t dajilog_board_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_0,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TRIPLE_PARTIAL,
        .touch_flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    return bsp_display_start_with_config(&cfg) != NULL ? ESP_OK : ESP_FAIL;
}

esp_err_t dajilog_board_backlight_on(void)
{
    return bsp_display_backlight_on();
}

bool dajilog_board_display_lock(int timeout_ms)
{
    /*
     * BSP 1.0.4 declares bsp_display_lock() as bool but returns the
     * esp_err_t value from esp_lv_adapter_lock() directly. ESP_OK is zero,
     * so a successful lock is incorrectly converted to false. Bypass that
     * wrapper until the BSP fixes its return-value conversion.
     */
    return esp_lv_adapter_lock(timeout_ms) == ESP_OK;
}

void dajilog_board_display_unlock(void)
{
    bsp_display_unlock();
}
