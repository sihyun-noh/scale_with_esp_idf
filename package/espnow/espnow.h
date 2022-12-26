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

#ifndef ESPNOW_H
#define ESPNOW_H

// #define CONFIG_ESPNOW_WIFI_MODE_STATION 1
// # CONFIG_ESPNOW_WIFI_MODE_STATION_SOFTAP is not set
#define CONFIG_ESPNOW_PMK "pmk1234567890123"
#define CONFIG_ESPNOW_LMK "lmk1234567890123"
#define CONFIG_ESPNOW_CHANNEL 1
#define CONFIG_ESPNOW_SEND_COUNT 100
#define CONFIG_ESPNOW_SEND_DELAY 1000
#define CONFIG_ESPNOW_SEND_LEN 10
#define CONFIG_ESPNOW_ENABLE_LONG_RANGE 1

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
// #if CONFIG_ESPNOW_WIFI_MODE_STATION
// #define ESPNOW_WIFI_MODE WIFI_MODE_STA
// #define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
// #else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
// #endif

#define ESPNOW_QUEUE_SIZE 6

#define IS_BROADCAST_ADDR(addr) (memcmp(addr, s_broadcast_mac, ESP_NOW_ETH_ALEN) == 0)

typedef enum {
  ESPNOW_SEND_CB,
  ESPNOW_RECV_CB,
} espnow_event_id_t;

typedef struct {
  uint8_t mac_addr[ESP_NOW_ETH_ALEN];
  esp_now_send_status_t status;
} espnow_event_send_cb_t;

typedef struct {
  uint8_t mac_addr[ESP_NOW_ETH_ALEN];
  uint8_t *data;
  int data_len;
} espnow_event_recv_cb_t;

typedef union {
  espnow_event_send_cb_t send_cb;
  espnow_event_recv_cb_t recv_cb;
} espnow_event_info_t;

/* When ESPNOW sending or receiving callback function is called, post event to ESPNOW task. */
typedef struct {
  espnow_event_id_t id;
  espnow_event_info_t info;
} espnow_event_t;

enum {
  ESPNOW_DATA_BROADCAST,
  ESPNOW_DATA_UNICAST,
  ESPNOW_DATA_MAX,
};

/* User defined field of ESPNOW data in this example. */
typedef struct {
  uint8_t type;        // Broadcast or unicast ESPNOW data.
  uint8_t state;       // Indicate that if has received broadcast ESPNOW data or not.
  uint16_t seq_num;    // Sequence number of ESPNOW data.
  uint16_t crc;        // CRC16 value of ESPNOW data.
  uint32_t magic;      // Magic number which is used to determine which device to send unicast ESPNOW data.
  uint8_t payload[0];  // Real payload of ESPNOW data.
} __attribute__((packed)) espnow_data_t;

/* Parameters of sending ESPNOW data. */
typedef struct {
  bool unicast;     // Send unicast ESPNOW data.
  bool broadcast;   // Send broadcast ESPNOW data.
  uint8_t state;    // Indicate that if has received broadcast ESPNOW data or not.
  uint32_t magic;   // Magic number which is used to determine which device to send unicast ESPNOW data.
  uint16_t count;   // Total count of unicast ESPNOW data to be sent.
  uint16_t delay;   // Delay between sending two ESPNOW data, unit: ms.
  int len;          // Length of ESPNOW data to be sent, unit: byte.
  uint8_t *buffer;  // Buffer pointing to ESPNOW data.
  uint8_t dest_mac[ESP_NOW_ETH_ALEN];  // MAC address of destination device.
} espnow_send_param_t;

void wifi_init(void);

void espnow_deinit(espnow_send_param_t *send_param);

esp_err_t espnow_init(void);

int start_esp_now(void);

void send_msg_to_esp_now(uint8_t *data, uint32_t len);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _ESP_NOW_H_ */
