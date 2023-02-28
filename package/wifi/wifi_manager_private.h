#ifndef _WIFI_MANAGER_PRIV_H_
#define _WIFI_MANAGER_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BSSID_LEN 6
#define MAX_SSID 33

typedef struct {
  uint8_t bssid[BSSID_LEN];
  uint8_t ssid[MAX_SSID];
  uint8_t primary_channel;
  int rssi;
} ap_info_t;

typedef enum {
  WIFI_OP_AP = 1,
  WIFI_OP_STA,
  WIFI_OP_AP_STA,
} wifi_op_mode_t;

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_MANAGER_PRIV_H_ */
