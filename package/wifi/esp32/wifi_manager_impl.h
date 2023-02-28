#ifndef _WIFI_MANAGER_IMPL_H_
#define _WIFI_MANAGER_IMPL_H_

#include "esp_wifi.h"
#include "wifi_manager_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AP_MODE 1
#define STA_MODE 2

#define WIFI_SCAN_FAILED (-1)
#define WIFI_SCAN_RUNNING (-2)

typedef struct wifi_context wifi_context_t;

/**
 * @brief Initialize the underlying ESP32 WiFi
 *
 * @return wifi_context_t* context of the WiFi
 */
wifi_context_t *wifi_init_impl();

/**
 * @brief Deinitialize the ESP32 Wifi component
 *
 * @param ctx the context of the WiFi
 */
void wifi_deinit_impl(wifi_context_t *ctx);

/**
 * @brief Set the ESP32 WiFi AP operating mode
 *
 * @param ctx the context of the WiFi
 * @param ssid of ESP32 soft-AP
 * @param password of ESP32 soft-AP
 * @return int 0 on success, -1 on failure
 */
int wifi_ap_mode_impl(wifi_context_t *ctx, const char *ssid, const char *password);

/**
 * @brief Set the ESP32 WiFi Station operating mode
 *
 * @param ctx the context of the WiFi
 * @return int 0 on success, -1 on failure
 */
int wifi_sta_mode_impl(wifi_context_t *ctx);

/**
 * @brief Set the ESP32 WiFi AP-Station operating mode
 *
 * @param ctx the context of the WiFi
 * @return int 0 on success, -1 on failure
 */
int wifi_ap_sta_mode_impl(wifi_context_t *ctx);

/**
 * @brief Stop WiFi If mode is WIFI_MODE_STA, it stop station and free station control block
 * If mode is WIFI_MODE_AP, it stop soft-AP and free soft-AP control block
 *
 * @param ctx the context of the WiFi
 * @return int 0 on success, -1 on failure
 */
int wifi_stop_mode_impl(wifi_context_t *ctx);

/**
 * @brief Set the ESP32 WiFi AP connect operating mode
 *
 * @param ctx the context of the WiFi
 * @param ssid Name of the network to connect to
 * @param password Security passphrase to connect to the network
 * @return int 0 on success, -1 on failure
 */
int wifi_connect_ap_impl(wifi_context_t *ctx, const char *ssid, const char *password);

/**
 * @brief Disconnect the ESP32 WiFi station from the AP.
 *
 * @param ctx the context of the WiFi
 * @return int 0 on success, -1 on failure
 */
int wifi_disconnect_ap_impl(wifi_context_t *ctx);

/**
 * @brief ESP32 Scan all available APs.
 *
 * @param ctx the context of the WiFi
 * @param userdata configuration of scanning
 * @param block if block is true, this API will block the caller until the scan is done, otherwise it will return
 * immediately
 * @param waitSec Inactive time. Unit seconds.
 * @return int 0 on success, -1 on failure
 */
int wifi_scan_network_impl(wifi_context_t *ctx, bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan,
                           uint8_t channel, const char *ssid, const uint8_t *bssid);

int get_wifi_scan_status_impl(wifi_context_t *ctx);

wifi_ap_record_t *get_wifi_scan_list_impl(wifi_context_t *ctx, uint16_t *scan_num);

/**
 * @brief Get current wifi mode (AP or Station)
 *
 * @param ctx the context of the WiFi
 * @return WIFI_MODE_STA on station mode, WIFI_MODE_AP on ap mode.
 */
int wifi_get_current_mode_impl(wifi_context_t *ctx);

/**
 * @brief Get device(station) ip address
 *
 * @param ctx the context of the WiFi
 * @param buffer of device's ip address
 * @param length of ip address buffer
 *
 * @return int 0 on success, -1 on failure
 */
int get_sta_ipaddr_impl(wifi_context_t *ctx, char *ip_addr, int addr_len);

/**
 * @brief Get router ip address
 *
 * @param ctx the context of the WiFi
 * @param buffer of router's ip address
 * @param length of ip address buffer
 *
 * @return int 0 on success, -1 on failure
 */
int get_router_ipaddr_impl(wifi_context_t *ctx, char *ip_addr, int addr_len);

/**
 * @brief Get router(ap) information
 *
 * @param ctx the context of the WiFi
 * @param router(ap) information structure, wifi_ap_record_t
 *
 * @return int 0 on success, -1 on failure
 */
int get_ap_info_impl(wifi_context_t *ctx, ap_info_t *ap_info);

/**
 * @brief Set the ESP32 WiFi ESP-NOW operating mode
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_espnow_mode_impl(wifi_context_t *ctx, wifi_op_mode_t wifi_op_mode);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_MANAGER_IMPL_H_ */
