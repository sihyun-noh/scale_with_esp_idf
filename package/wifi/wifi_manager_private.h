#ifndef _WIFI_MANAGER_PRIV_H_
#define _WIFI_MANAGER_PRIV_H_

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

#endif /* _WIFI_MANAGER_PRIV_H_ */
