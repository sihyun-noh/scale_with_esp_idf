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
#include "comm_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STATION 0
#define SOFTAP 1
#define CONFIG_ESPNOW_WIFI_MODE SOFTAP

#define CONFIG_ESPNOW_CHANNEL 1

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE == STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#elif CONFIG_ESPNOW_WIFI_MODE == SOFTAP
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
int espnow_remove_peers(device_t device_mode);
bool espnow_remove_peer(const uint8_t *mac_addr);
int espnow_list_peers(peer_info_t *peers, int max_peers);
bool espnow_has_peer(const uint8_t *mac_addr);
bool espnow_send_data(const uint8_t *mac_addr, const uint8_t *data, size_t len);
uint8_t *espnow_get_master_addr(void);

unsigned int ascii_to_hex(const char *str, size_t size, uint8_t *hex);

/**
 * @brief Get mac address (peer address) of requested device type
 * @param dev, device type such as MAIN, HID, CHILD
 * @param mac_address, mac address of device type
 * @return true on success, false on failure
 */
bool get_mac_address(device_type_t dev, uint8_t *mac_address);

/**
 * @brief Set mac address (peer address) of device type to the syscfg
 * @param dev, device type
 * @param mac_address, mac address of device type
 */
void set_mac_address(device_type_t dev, char *mac_address);

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
