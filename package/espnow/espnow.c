/**
 * @file esp_now.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_mac.h"

#include "espnow.h"
#include "log.h"
#include "sys_config.h"

static const char *TAG = "ESP-NOW";
static bool b_ready = false;
static bool b_register_peers = false;

#define HID_CHILD_CNT 7

static char *device_peer_list[8] = {
  SYSCFG_N_HID_MAC,    SYSCFG_N_CHILD1_MAC, SYSCFG_N_CHILD2_MAC, SYSCFG_N_CHILD3_MAC,
  SYSCFG_N_CHILD4_MAC, SYSCFG_N_CHILD5_MAC, SYSCFG_N_CHILD6_MAC, SYSCFG_N_MASTER_MAC
};

uint8_t macAddress[HID_CHILD_CNT][MAC_ADDR_LEN] = { 0 };  // 0 = HID, 1~6 = child 1~6
uint8_t masterAddress[MAC_ADDR_LEN] = { 0 };

bool syscfg_get_to_add_peer(const char *key, uint8_t *mac_addr);

uint8_t *espnow_get_master_addr(void) {
  if (!b_register_peers) {
    if (syscfg_get_to_add_peer(SYSCFG_N_MASTER_MAC, masterAddress)) {
      LOGI(TAG, "Success to get master addr");
      LOG_BUFFER_HEXDUMP(TAG, masterAddress, sizeof(masterAddress), LOG_INFO);
    } else {
      LOGI(TAG, "Failed to get master addr");
    }
  }
  return masterAddress;
}

bool espnow_start(esp_now_recv_cb_t recv_cb, esp_now_send_cb_t send_cb) {
  espnow_stop();

  /* Initialize ESPNOW and register sending and receiving callback function. */
  b_ready = esp_now_init() == 0 && esp_now_register_recv_cb(recv_cb) == 0 && esp_now_register_send_cb(send_cb) == 0;

  return b_ready;
}

void espnow_stop(void) {
  if (!b_ready) {
    return;
  }
  esp_now_deinit();
  b_ready = false;
}

unsigned int ascii_to_hex(const char *str, size_t size, uint8_t *hex) {
  unsigned int i, h, high, low;
  for (h = 0, i = 0; i < size; i += 2, ++h) {
    high = (str[i] > '9') ? str[i] - 'a' + 10 : str[i] - '0';
    low = (str[i + 1] > '9') ? str[i + 1] - 'a' + 10 : str[i + 1] - '0';
    hex[h] = (high << 4) | low;
  }
  return h;
}

int get_address_matching_id(void) {
  char s_compare_mac[SYSCFG_S_MASTER_MAC] = { 0 };
  uint8_t my_mac[ESP_NOW_ETH_ALEN] = { 0 };
  uint8_t compare_mac[ESP_NOW_ETH_ALEN] = { 0 };
#if (CONFIG_ESPNOW_WIFI_MODE == STATION)
  esp_read_mac(my_mac, ESP_MAC_WIFI_STA);
#elif (CONFIG_ESPNOW_WIFI_MODE == SOFTAP)
  esp_read_mac(my_mac, ESP_MAC_WIFI_SOFTAP);
#endif

  for (int i = 0; i < 8; i++) {
    if (syscfg_get(MFG_DATA, device_peer_list[i], s_compare_mac, sizeof(s_compare_mac))) {
      LOGW(TAG, "syscfg MAC Address read error");
      return -1;
    }
    ascii_to_hex(s_compare_mac, sizeof(compare_mac) * 2, compare_mac);
    if (memcmp(my_mac, compare_mac, sizeof(my_mac)) == 0) {
      if (i == 0) {  // hid = 10
        i = 10;
      } else if (i >= 7) {  // master = 0
        i = 0;
      }
      return i;
    }
  }
  return -1;
}

bool syscfg_get_to_add_peer(const char *key, uint8_t *mac_addr) {
  char s_mac[SYSCFG_S_MASTER_MAC] = { 0 };

  if (syscfg_get(MFG_DATA, key, s_mac, sizeof(s_mac))) {
    LOGW(TAG, "syscfg MAC Address read error");
    return false;
  }
  ascii_to_hex(s_mac, MAC_ADDR_LEN * 2, mac_addr);
  LOG_BUFFER_HEXDUMP(TAG, mac_addr, MAC_ADDR_LEN, LOG_INFO);
  return espnow_add_peer(mac_addr, CONFIG_ESPNOW_CHANNEL, ESPNOW_WIFI_IF);
}

// TODO : Implement to read the device's mac address from MFG data
// and then add mac address to the peer list in accordance with device type
// Add mac address read from the MFG data of each devices to the esp now peer list
bool espnow_add_peers(device_t device_mode) {
  b_register_peers = false;

  if (!b_ready) {
    return b_register_peers;
  }

  switch (device_mode) {
    case HID_DEVICE:
    case CHILD_DEVICE:
      // add main mac address to the peer list
      if (syscfg_get_to_add_peer(SYSCFG_N_MASTER_MAC, masterAddress)) {
        b_register_peers = true;
      } else {
        LOGI(TAG, "Add peer Error");
      }
      break;
    case MASTER_DEVICE:
      // add hid and all child mac address to the peer list
      uint8_t broadcastAddress[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      espnow_add_peer(broadcastAddress, CONFIG_ESPNOW_CHANNEL, ESPNOW_WIFI_IF);

      for (int i = 0; i < HID_CHILD_CNT; i++) {
        if (syscfg_get_to_add_peer(device_peer_list[i], macAddress[i])) {
          b_register_peers = true;
        } else {
          LOGI(TAG, "%s device seems to be removed", device_peer_list[i]);
        }
      }
      break;
  }
  return b_register_peers;
}

bool espnow_add_peer(const uint8_t *mac_addr, uint8_t channel, int netif) {
  if (!b_ready) {
    return false;
  }

  esp_now_peer_info_t peer;
  memset(&peer, 0, sizeof(esp_now_peer_info_t));
  peer.channel = channel;
  peer.ifidx = netif;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_addr, MAC_ADDR_LEN);

  if (espnow_has_peer(mac_addr)) {
    return esp_now_mod_peer(&peer) == ESP_OK;
  }
  return esp_now_add_peer(&peer) == ESP_OK;
}

int espnow_list_peers(peer_info_t *peers, int max_peers) {
  if (!b_ready || !peers) {
    return 0;
  }

  int peer_cnt = 0;
  esp_now_peer_info_t peer;
  for (esp_err_t e = esp_now_fetch_peer(true, &peer); e == ESP_OK; e = esp_now_fetch_peer(false, &peer)) {
    uint8_t *mac_addr = peer.peer_addr;
    uint8_t channel = peer.channel;
    if (peer_cnt < max_peers) {
      memcpy(peers[peer_cnt].mac_addr, mac_addr, MAC_ADDR_LEN);
      peers[peer_cnt].channel = channel;
    }
    peer_cnt++;
  }
  return peer_cnt;
}

int espnow_remove_peers(device_t device_mode) {
  if (!b_ready) {
    return 0;
  }

  LOGI(TAG, "Call espnow_remove_peers()");

  int peer_cnt = 0;
  char mac_addr[13];
  uint8_t peer_addr[MAC_ADDR_LEN];

  switch (device_mode) {
    case HID_DEVICE:
    case CHILD_DEVICE: {
    } break;
    case MASTER_DEVICE: {
      // Remove only child mac address
      for (int i = 1; i <= 6; i++) {
        memset(mac_addr, 0x00, sizeof(mac_addr));
        memset(peer_addr, 0x00, sizeof(peer_addr));
        if (syscfg_get(MFG_DATA, device_peer_list[i], mac_addr, sizeof(mac_addr)) == 0) {
          ascii_to_hex(mac_addr, MAC_ADDR_LEN * 2, peer_addr);
          esp_now_del_peer(peer_addr);
          syscfg_unset(MFG_DATA, device_peer_list[i]);
          peer_cnt++;
        }
      }
    } break;
  }
  return peer_cnt;
}

bool espnow_has_peer(const uint8_t *mac_addr) {
  return b_ready && esp_now_is_peer_exist(mac_addr);
}

bool espnow_remove_peer(const uint8_t *mac_addr) {
  return b_ready && esp_now_del_peer(mac_addr);
}

bool espnow_send_data(const uint8_t *mac_addr, const uint8_t *data, size_t len) {
  if (!b_ready || len > MAX_MSG_LEN || len == 0) {
    return false;
  }
  return esp_now_send(mac_addr, data, len) == ESP_OK;
}

bool get_mac_address(device_type_t dev, uint8_t *mac_address) {
  char mac[SYSCFG_S_MASTER_MAC] = { 0 };
  char *key = device_peer_list[(int)dev];

  if (syscfg_get(MFG_DATA, key, mac, sizeof(mac))) {
    LOGW(TAG, "Failed to get key = %s from syscfg", key);
    return false;
  }

  ascii_to_hex(mac, MAC_ADDR_LEN * 2, mac_address);
  return true;
}

void set_mac_address(device_type_t dev, char *mac_address) {
  char *key = device_peer_list[(int)dev];

  if (syscfg_set(MFG_DATA, key, mac_address)) {
    LOGW(TAG, "Failed to save key = %s, mac address = %s to syscfg", key, mac_address);
  } else {
    LOGI(TAG, "Success to save key = %s, mac address = %s to syscfg", key, mac_address);
  }
}
