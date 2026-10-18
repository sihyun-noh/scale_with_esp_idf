#include "dajilog_board.h"

#include "bsp/esp-bsp.h"
#include "esp_log.h"

static const char *TAG = "DajilogBoard7B";

#ifdef CONFIG_DAJILOG_CAN_ENABLE
#define DAJILOG_BOARD_HAS_CAN true
#define DAJILOG_BOARD_CAN_TX_GPIO CONFIG_DAJILOG_CAN_TX_GPIO
#define DAJILOG_BOARD_CAN_RX_GPIO CONFIG_DAJILOG_CAN_RX_GPIO
#define DAJILOG_BOARD_CAN_ENABLE_GPIO CONFIG_DAJILOG_CAN_TRANSCEIVER_ENABLE_GPIO
#else
#define DAJILOG_BOARD_HAS_CAN false
#define DAJILOG_BOARD_CAN_TX_GPIO (-1)
#define DAJILOG_BOARD_CAN_RX_GPIO (-1)
#define DAJILOG_BOARD_CAN_ENABLE_GPIO (-1)
#endif

static const dajilog_board_info_t s_board_info = {
    .board_name = "Waveshare ESP32-P4-WIFI6-Touch-LCD-7B",
    .display_width = 1024,
    .display_height = 600,
    .ui_profile = DAJILOG_UI_PROFILE_LANDSCAPE_1024_600,
    .has_touch = true,
    .has_wifi = true,
    .has_usb_host = true,
    .has_can = DAJILOG_BOARD_HAS_CAN,
    .has_rs485 = false,
    .has_sdi12 = false,
};

static const dajilog_board_can_config_t s_can_config = {
    .supported = DAJILOG_BOARD_HAS_CAN,
    .tx_gpio = DAJILOG_BOARD_CAN_TX_GPIO,
    .rx_gpio = DAJILOG_BOARD_CAN_RX_GPIO,
    .transceiver_enable_gpio = DAJILOG_BOARD_CAN_ENABLE_GPIO,
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
        .rotation = ESP_LV_ADAPTER_ROTATE_180,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DOUBLE_DIRECT,
        .touch_flags = {
            .swap_xy = 0,
            /* The display bridge already applies the configured 180-degree rotation. */
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    if (bsp_display_start_with_config(&cfg) == NULL) {
        return ESP_FAIL;
    }

    lv_indev_t *touch = bsp_display_get_input_dev();
    if (touch == NULL) {
        ESP_LOGE(TAG, "Touch input device is unavailable");
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}

esp_err_t dajilog_board_backlight_on(void)
{
    return bsp_display_backlight_on();
}

bool dajilog_board_display_lock(int timeout_ms)
{
    return bsp_display_lock((uint32_t)timeout_ms);
}

void dajilog_board_display_unlock(void)
{
    bsp_display_unlock();
}
