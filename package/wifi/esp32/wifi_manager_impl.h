#ifndef _WIFI_MANAGER_IMPL_H_
#define _WIFI_MANAGER_IMPL_H_

#include "wifi_manager.h"

#define AP_MODE 1
#define STA_MODE 2

typedef struct wifi_context wifi_context_t;

/**
 * @brief Initialize the underlying ESP32 Wi-fi
 *
 * @return wifi_context_t* context of the Wi-Fi
 */
wifi_context_t *wifi_init_impl();

/**
 * @brief Deinitialize the ESP32 Wi-fi component
 *
 * @param ctx the context of the Wi-Fi
 */
void wifi_deinit_impl(wifi_context_t *ctx);

/**
 * @brief Set the ESP32 Wi-Fi AP operating mode
 *
 * @param ctx the context of the Wi-Fi
 * @param ssid of ESP32 soft-AP
 * @param password of ESP32 soft-AP
 * @return int 0 on success, -1 on failure
 */
int wifi_ap_mode_impl(wifi_context_t *ctx, const char *ssid, const char *password);

/**
 * @brief Set the ESP32 Wi-Fi Station operating mode
 *
 * @param ctx the context of the Wi-Fi
 * @return int 0 on success, -1 on failure
 */
int wifi_sta_mode_impl(wifi_context_t *ctx);

/**
 * @brief Stop WiFi If mode is WIFI_MODE_STA, it stop station and free station control block
 * If mode is WIFI_MODE_AP, it stop soft-AP and free soft-AP control block
 *
 * @param ctx the context of the Wi-Fi
 * @return int 0 on success, -1 on failure
 */
int wifi_stop_mode_impl(wifi_context_t *ctx);

/**
 * @brief Set the ESP32 Wi-Fi AP connect operating mode
 *
 * @param ctx the context of the Wi-Fi
 * @param ssid Name of the network to connect to
 * @param password Security passphrase to connect to the network
 * @return int 0 on success, -1 on failure
 */
int wifi_connect_ap_impl(wifi_context_t *ctx, const char *ssid, const char *password);

/**
 * @brief Disconnect the ESP32 Wi-Fi station from the AP.
 *
 * @param ctx the context of the Wi-Fi
 * @return int 0 on success, -1 on failure
 */
int wifi_disconnect_ap_impl(wifi_context_t *ctx);

/**
 * @brief ESP32 Scan all available APs.
 *
 * @param ctx the context of the Wi-Fi
 * @param userdata configuration of scanning
 * @param block if block is true, this API will block the caller until the scan is done, otherwise it will return
 * immediately
 * @param waitSec Inactive time. Unit seconds.
 * @return int 0 on success, -1 on failure
 */
int wifi_scan_network_impl(wifi_context_t *ctx, scan_network_result_t *userdata, bool block, int waitSec);

#endif /* _WIFI_MANAGER_IMPL_H_ */
