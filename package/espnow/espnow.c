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

static const char *TAG = "ESP-NOW";
static bool b_ready = false;

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

// TODO : Implement to read the device's mac address from MFG data
// and then add mac address to the peer list in accordance with device type
// Add mac address read from the MFG data of each devices to the esp now peer list
int espnow_add_peers(device_t device_mode) {
  switch (device_mode) {
    case HID_DEVICE:
      // add main mac address to the peer list
      break;
    case MAIN_DEVICE:
      // add hid and all child mac address to the peer list
      break;
    case CHILD_DEVICE:
      // add main mac address to the peer list
      break;
  }

  return 0;
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

bool espnow_has_peer(const uint8_t *mac_addr) {
  return b_ready && esp_now_is_peer_exist(mac_addr);
}

bool espnow_remove_peer(const uint8_t *mac_addr) {
  return b_ready && esp_now_del_peer(mac_addr);
}

int espnow_send_data(const uint8_t *mac_addr, const uint8_t *data, size_t len) {
  if (!b_ready || len > MAX_MSG_LEN || len == 0) {
    return false;
  }
  return esp_now_send(mac_addr, data, len) == ESP_OK;
}
