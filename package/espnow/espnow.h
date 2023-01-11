/**
 * @file espnow.h
 *
 * @brief Implement generic ESP-NOW API that will be used by the ESP-NOW client
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef _ESP_NOW_H_
#define _ESP_NOW_H_

#include <stdint.h>
#include <stdbool.h>

#include "esp_now.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define CONFIG_ESPNOW_WIFI_MODE_STATION 1
// # CONFIG_ESPNOW_WIFI_MODE_STATION_SOFTAP is not set
#define CONFIG_ESPNOW_CHANNEL 1

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
#endif

#define MAX_MSG_LEN ESP_NOW_MAX_DATA_LEN
#define MAC_ADDR_LEN ESP_NOW_ETH_ALEN

typedef enum { HID_DEVICE = 0, MASTER_DEVICE, CHILD_DEVICE } device_t;

typedef struct {
  uint8_t mac_addr[MAC_ADDR_LEN];
  uint8_t channel;
} peer_info_t;

bool espnow_start(esp_now_recv_cb_t recv_cb, esp_now_send_cb_t send_cb);
void espnow_stop(void);
bool espnow_add_peers(device_t device_mode);
bool espnow_add_peer(const uint8_t *mac_addr, uint8_t channel, int netif);
bool espnow_remove_peer(const uint8_t *mac_addr);
int espnow_list_peers(peer_info_t *peers, int max_peers);
bool espnow_has_peer(const uint8_t *mac_addr);
int espnow_send_data(const uint8_t *mac_addr, const uint8_t *data, size_t len);
uint8_t *espnow_get_master_addr(void);

/**
 * @brief Find the ID matched with the Mac Address
 *
 * @return 0 = master ID, 1~6 = child ID, 10 = HID ID
 */
int get_address_matching_id(void);
#ifdef __cplusplus
}
#endif

#endif /* _ESP_NOW_H_ */
