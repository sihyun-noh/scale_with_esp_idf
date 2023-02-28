#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

#include "wifi_manager_private.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the underlying Wi-fi
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_user_init(void);

/**
 * @brief Deinitialize the Wi-fi component
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_user_deinit(void);

/**
 * @brief Set the WiFi AP operating mode
 *
 * @param ssid of ESP32 soft-AP
 * @param password of ESP32 soft-AP
 * @return int 0 on success, -1 on failure
 */
int wifi_ap_mode(const char *ssid, const char *password);

/**
 * @brief Set the WiFi Station operating mode
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_sta_mode(void);

/**
 * @brief Set the WiFi AP+STA operating mode
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_ap_sta_mode(void);

/**
 * @brief Stop WiFi If mode is WIFI_MODE_STA, it stop station and free station control block If mode is WIFI_MODE_AP, it
 * stop soft-AP and free soft-AP control block If mode is WIFI_MODE_APSTA, it stop station/soft-AP and free
 * station/soft-AP control block.
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_stop_mode(void);

/**
 * @brief Set the WiFi AP connect operating mode
 *
 * @param ssid Name of the network to connect to
 * @param password Security passphrase to connect to the network
 * @return int 0 on success, -1 on failure
 */
int wifi_connect_ap(const char *ssid, const char *password);

/**
 * @brief Disconnect the WiFi station from the AP.
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_disconnect_ap(void);

/**
 * @brief Scan all available APs.
 *
 * @param userdata configuration of scanning
 * @param block if block is true, this API will block the caller until the scan is done, otherwise it will return
 * immediately
 * @param waitSec Inactive time. Unit seconds.
 * @return int 0 on success, -1 on failure
 */
int wifi_scan_network(bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan, uint8_t channel,
                      const char *ssid, const uint8_t *bssid);

int get_wifi_scan_status(void);

ap_info_t *get_wifi_scan_list(uint16_t *scan_num);

/**
 * @brief Get current wifi mode (AP or Station)
 *
 * @return WIFI_MODE_STA on station mode, WIFI_MODE_AP on ap mode.
 */
int wifi_get_current_mode(void);

/**
 * @brief Get device(station) ip address
 *
 * @param buffer of device's ip address
 * @param length of ip address buffer
 *
 * @return int 0 on success, -1 on failure
 */
int get_sta_ipaddr(char *ip_addr, int addr_len);

/**
 * @brief Get router ip address
 *
 * @param buffer of router's ip address
 * @param length of ip address buffer
 *
 * @return int 0 on success, -1 on failure
 */
int get_router_ipaddr(char *ip_addr, int addr_len);

/**
 * @brief Get router information (ssid, rssi)
 *
 * @param router information structure (ssid, rssi)
 *
 * @return int 0 on success, -1 on failure
 */
int get_ap_info(ap_info_t *ap_info);

/**
 * @brief Initialize the espnow Wi-fi
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_espnow_mode(wifi_op_mode_t wifi_op_mode);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_MANAGER_H_ */
