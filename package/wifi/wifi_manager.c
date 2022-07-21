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

static wifi_context_t *ctx = NULL;

int wifi_user_init() {
  ctx = wifi_init_impl();
  if (ctx == NULL) {
    return -1;
  }
  return 0;
}

int wifi_user_deinit() {
  if (ctx) {
    wifi_deinit_impl(ctx);
  }
  return 0;
}

int wifi_ap_mode(const char *ssid, const char *password) {
  return wifi_ap_mode_impl(ctx, ssid, password);
}

int wifi_sta_mode() {
  return wifi_sta_mode_impl(ctx);
}

int wifi_stop_mode() {
  return wifi_stop_mode_impl(ctx);
}

int wifi_connect_ap(const char *ssid, const char *password) {
  return wifi_connect_ap_impl(ctx, ssid, password);
}

int wifi_disconnect_ap() {
  return wifi_disconnect_ap_impl(ctx);
}

int wifi_scan_network(scan_network_result_t *userdata, bool block, int waitSec) {
  return wifi_scan_network_impl(ctx, userdata, block, waitSec);
}

int wifi_get_current_mode() {
  return wifi_get_current_mode_impl(ctx);
}

int get_sta_ipaddr(char *ip_addr, int addr_len) {
  return get_sta_ipaddr_impl(ctx, ip_addr, addr_len);
}

int get_router_ipaddr(char *ip_addr, int addr_len) {
  return get_router_ipaddr_impl(ctx, ip_addr, addr_len);
}

int get_ap_info(ap_info_t *ap_info) {
  wifi_ap_record_t ap_record = { 0 };

  esp_err_t rc = get_ap_info_impl(ctx, &ap_record);

  if (rc == ESP_OK) {
    snprintf(ap_info->ssid, sizeof(ap_info->ssid), "%s", ap_record.ssid);
    ap_info->rssi = ap_record.rssi;
  }
  return rc;
}
