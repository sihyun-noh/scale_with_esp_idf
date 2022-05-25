#ifndef _WIFI_MANAGER_H_
#define _WIFI_MANAGER_H_

#include <stdbool.h>

/**
 * @brief Initialize the underlying Wi-fi
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_user_init();

/**
 * @brief Deinitialize the Wi-fi component
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_user_deinit();

/**
 * @brief Set the Wi-Fi AP operating mode
 *
 * @param ssid of ESP32 soft-AP
 * @param password of ESP32 soft-AP
 * @return int 0 on success, -1 on failure
 */
int wifi_ap_mode(const char *ssid, const char *password);

/**
 * @brief Set the Wi-Fi Station operating mode
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_sta_mode();

/**
 * @brief Stop WiFi If mode is WIFI_MODE_STA, it stop station and free station control block If mode is WIFI_MODE_AP, it
 * stop soft-AP and free soft-AP control block If mode is WIFI_MODE_APSTA, it stop station/soft-AP and free
 * station/soft-AP control block.
 *
 * @return int 0 on success, -1 on failure
 */
int wifi_stop_mode();

/**
 * @brief Set the Wi-Fi AP connect operating mode
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
int wifi_disconnect_ap();

typedef void (*scan_done_handler_t)(void *userdata);

/**
 * @brief The context of the Wi-Fi Network Scan
 */
typedef struct scan_network_result {
  int scan_done;
  int scan_ap_num;
  int scan_ap_req_count;
  void *scan_ap_list;
  scan_done_handler_t scan_done_handler;
} scan_network_result_t;

/**
 * @brief Scan all available APs.
 *
 * @param userdata configuration of scanning
 * @param block if block is true, this API will block the caller until the scan is done, otherwise it will return
 * immediately
 * @param waitSec Inactive time. Unit seconds.
 * @return int 0 on success, -1 on failure
 */
int wifi_scan_network(scan_network_result_t *userdata, bool block, int waitSec);

#endif /* _WIFI_MANAGER_H_ */
