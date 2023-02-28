/**
 * @file wifi_manager.c
 *
 * @brief Wi-Fi abstraction api that will be used in application layer
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "esp32/wifi_manager_impl.h"
#include "wifi_manager.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static wifi_context_t *ctx = NULL;

int wifi_user_init(void) {
  ctx = wifi_init_impl();
  if (ctx == NULL) {
    return -1;
  }
  return 0;
}

int wifi_user_deinit(void) {
  if (ctx) {
    wifi_deinit_impl(ctx);
  }
  return 0;
}

int wifi_ap_mode(const char *ssid, const char *password) {
  return wifi_ap_mode_impl(ctx, ssid, password);
}

int wifi_sta_mode(void) {
  return wifi_sta_mode_impl(ctx);
}

int wifi_ap_sta_mode(void) {
  return wifi_ap_sta_mode_impl(ctx);
}

int wifi_stop_mode(void) {
  return wifi_stop_mode_impl(ctx);
}

int wifi_connect_ap(const char *ssid, const char *password) {
  return wifi_connect_ap_impl(ctx, ssid, password);
}

int wifi_disconnect_ap(void) {
  return wifi_disconnect_ap_impl(ctx);
}

int wifi_scan_network(bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan, uint8_t channel,
                      const char *ssid, const uint8_t *bssid) {
  return wifi_scan_network_impl(ctx, async, show_hidden, passive, max_ms_per_chan, channel, ssid, bssid);
}

int get_wifi_scan_status(void) {
  return get_wifi_scan_status_impl(ctx);
}

ap_info_t *get_wifi_scan_list(uint16_t *scan_num) {
  uint16_t scan_ap_num = 0;
  wifi_ap_record_t *ap_record = get_wifi_scan_list_impl(ctx, &scan_ap_num);

  *scan_num = scan_ap_num;

  ap_info_t *ap_info = calloc(scan_ap_num, sizeof(ap_info_t));
  if (ap_info) {
    for (int i = 0; i < scan_ap_num; i++) {
      memcpy(ap_info[i].bssid, ap_record[i].bssid, sizeof(ap_info[i].bssid));
      snprintf((char *)ap_info[i].ssid, sizeof(ap_info[i].ssid), "%s", (char *)ap_record[i].ssid);
      ap_info[i].rssi = ap_record[i].rssi;
      ap_info[i].primary_channel = ap_record[i].primary;
    }
  }

  return ap_info;
}

int wifi_get_current_mode(void) {
  return wifi_get_current_mode_impl(ctx);
}

int get_sta_ipaddr(char *ip_addr, int addr_len) {
  return get_sta_ipaddr_impl(ctx, ip_addr, addr_len);
}

int get_router_ipaddr(char *ip_addr, int addr_len) {
  return get_router_ipaddr_impl(ctx, ip_addr, addr_len);
}

int get_ap_info(ap_info_t *ap_info) {
  return get_ap_info_impl(ctx, ap_info);
}

int wifi_espnow_mode(wifi_op_mode_t wifi_op_mode) {
  return wifi_espnow_mode_impl(ctx, wifi_op_mode);
}
