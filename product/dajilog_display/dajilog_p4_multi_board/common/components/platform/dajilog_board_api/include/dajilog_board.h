#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DAJILOG_UI_PROFILE_PORTRAIT_720_1280 = 0,
    DAJILOG_UI_PROFILE_LANDSCAPE_1024_600,
    DAJILOG_UI_PROFILE_CUSTOM,
} dajilog_ui_profile_t;

typedef struct {
    const char *board_name;
    uint16_t display_width;
    uint16_t display_height;
    dajilog_ui_profile_t ui_profile;
    bool has_touch;
    bool has_wifi;
    bool has_usb_host;
    bool has_can;
    bool has_rs485;
    bool has_sdi12;
} dajilog_board_info_t;

typedef struct {
    bool supported;
    int tx_gpio;
    int rx_gpio;
    int transceiver_enable_gpio;
    uint32_t bitrate;
} dajilog_board_can_config_t;

const dajilog_board_info_t *dajilog_board_get_info(void);
const dajilog_board_can_config_t *dajilog_board_get_can_config(void);
esp_err_t dajilog_board_init(void);
esp_err_t dajilog_board_display_start(void);
esp_err_t dajilog_board_backlight_on(void);
bool dajilog_board_display_lock(int timeout_ms);
void dajilog_board_display_unlock(void);

#ifdef __cplusplus
}
#endif
