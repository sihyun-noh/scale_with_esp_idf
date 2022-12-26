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
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "syslog.h"
#include "esp_system.h"
#include "esp_crc.h"
#include "espnow.h"
#include "esp_random.h"
#include "esp_mac.h"
#include "sys_status.h"
#include "sysevent.h"
#include "event_ids.h"
#include "config.h"
#include "log.h"

static const char *TAG = "ESP-NOW";

espnow_send_param_t *send_param;
QueueHandle_t s_espnow_queue;
static TaskHandle_t espnow_task_handle = NULL;

int set_data;
#define ESPNOW_MAXDELAY 512

static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint16_t s_espnow_seq[ESPNOW_DATA_MAX] = { 0, 0 };
static uint8_t master_addr[ESP_NOW_ETH_ALEN] = { 0 };

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
  espnow_event_t evt;
  espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

  if (mac_addr == NULL) {
    LOGE(TAG, "Send cb arg error");
    return;
  }

  evt.id = ESPNOW_SEND_CB;
  memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
  send_cb->status = status;
  if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
    LOGW(TAG, "Send send queue fail");
  }
}

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
  espnow_event_t evt;
  espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

  if (mac_addr == NULL || data == NULL || len <= 0) {
    LOGE(TAG, "Receive cb arg error");
    return;
  }

  evt.id = ESPNOW_RECV_CB;
  memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
  recv_cb->data = malloc(len);
  if (recv_cb->data == NULL) {
    LOGE(TAG, "Malloc receive data fail");
    return;
  }
  memcpy(recv_cb->data, data, len);
  recv_cb->data_len = len;
  if (xQueueSend(s_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
    LOGW(TAG, "Send receive queue fail");
    free(recv_cb->data);
  }
}
char data_buf[250] = { 0 };

/* Parse received ESPNOW data. */
int espnow_data_parse(uint8_t *data, uint16_t data_len, uint8_t *state, uint16_t *seq, int *magic) {
  espnow_data_t *buf = (espnow_data_t *)data;
  uint16_t crc, crc_cal = 0;

  if (data_len < sizeof(espnow_data_t)) {
    LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
    return -1;
  }

  *state = buf->state;
  *seq = buf->seq_num;
  *magic = buf->magic;
  crc = buf->crc;
  buf->crc = 0;
  crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

  snprintf((char *)data_buf, data_len - sizeof(espnow_data_t) + 1, (char *)buf->payload);

  LOGI(TAG, "data len : %d", data_len - sizeof(espnow_data_t));
  LOGI(TAG, "data_buf : %s", data_buf);

  if (crc_cal == crc) {
    return buf->type;
  }

  return -1;
}

/* Prepare ESPNOW data to be sent. */
void espnow_data_prepare(espnow_send_param_t *send_param, uint8_t *data, uint32_t data_len) {
  espnow_data_t *buf = (espnow_data_t *)send_param->buffer;

  memcpy(buf->payload, data, data_len);
  send_param->len = sizeof(espnow_data_t) + data_len;

  assert(send_param->len >= sizeof(espnow_data_t));

  buf->type = IS_BROADCAST_ADDR(send_param->dest_mac) ? ESPNOW_DATA_BROADCAST : ESPNOW_DATA_UNICAST;
  buf->state = send_param->state;
  buf->seq_num = s_espnow_seq[buf->type]++;
  buf->crc = 0;
  buf->magic = send_param->magic;

  /* Fill all remaining bytes after the data with random values */
  // esp_fill_random(buf->payload, send_param->len - sizeof(espnow_data_t));
  buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

int add_peer_broadcast() {
  /* Add broadcast peer information to peer list. */
  esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
  if (peer == NULL) {
    LOGE(TAG, "Malloc peer information fail");
    vSemaphoreDelete(s_espnow_queue);
    esp_now_deinit();
    return -1;
  }
  memset(peer, 0, sizeof(esp_now_peer_info_t));
  peer->channel = CONFIG_ESPNOW_CHANNEL;
  peer->ifidx = ESPNOW_WIFI_IF;
  peer->encrypt = false;
  memcpy(peer->peer_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
  esp_now_add_peer(peer);
  free(peer);
  return 0;
}

int add_peer_unicast(const uint8_t *mac_addr) {
  /* If MAC address does not exist in peer list, add it to peer list. */
  if (esp_now_is_peer_exist(mac_addr) == false) {
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
      LOGE(TAG, "Malloc peer information fail");
      // espnow_deinit(send_param);
      return -1;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = true;
    memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
    memcpy(peer->peer_addr, mac_addr, ESP_NOW_ETH_ALEN);
    esp_now_add_peer(peer);
    free(peer);
  }
  return 0;
}

static void espnow_task(void *pvParameter) {
  espnow_event_t evt;
  uint8_t recv_state = 0;
  uint16_t recv_seq = 0;
  int recv_magic = 0;
  // bool is_broadcast = false;
  int ret;

  /* Start sending broadcast ESPNOW data. */
  espnow_send_param_t *send_param = (espnow_send_param_t *)pvParameter;

  // add_peer_broadcast();
  add_peer_unicast(master_addr);

  memcpy(send_param->dest_mac, master_addr, ESP_NOW_ETH_ALEN);

  vTaskDelay(5000 / portTICK_PERIOD_MS);

  for (;;) {
    if (set_data) {
      if (!esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len)) {
        // LOGE(TAG, "Send error");
        // espnow_deinit(send_param);
        // vTaskDelete(NULL);
      }
      // free(send_param->buffer);
      LOGI(TAG, "dest_mac : " MACSTR ", buf : %s, len = %d", MAC2STR(send_param->dest_mac), send_param->buffer,
           send_param->len);
      set_data = 0;
    }

    if (xQueueReceive(s_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
      switch (evt.id) {
        case ESPNOW_SEND_CB: {
          espnow_event_send_cb_t *send_cb = &evt.info.send_cb;
          // is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);

          LOGD(TAG, "Send data to " MACSTR ", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);
          /* Delay a while before sending the next data. */
          if (send_param->delay > 0) {
            vTaskDelay(send_param->delay / portTICK_PERIOD_MS);
          }

          LOGI(TAG, "send data to " MACSTR "", MAC2STR(send_cb->mac_addr));

          memcpy(send_param->dest_mac, send_cb->mac_addr, ESP_NOW_ETH_ALEN);

          LOGI(TAG, "dest_mac : " MACSTR ", buf : %s, len = %d", MAC2STR(send_param->dest_mac),
               (char *)send_param->buffer, send_param->len);
          break;
        }
        case ESPNOW_RECV_CB: {
          espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
          LOG_BUFFER_HEXDUMP(TAG, recv_cb->data, recv_cb->data_len, LOG_INFO);
          ret = espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic);
          free(recv_cb->data);
          LOGI(TAG, "Receive %dth data from: " MACSTR ", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr),
               recv_cb->data_len);
          LOGI(TAG, "receive data : %s", data_buf);
          if ((ret != ESPNOW_DATA_UNICAST) && (ret != ESPNOW_DATA_BROADCAST)) {
            LOGI(TAG, "Receive error data from: " MACSTR "", MAC2STR(recv_cb->mac_addr));
          }
          break;
        }
        default: LOGE(TAG, "Callback type error: %d", evt.id); break;
      }
    }
  }
  vTaskDelete(NULL);
  espnow_task_handle = NULL;
}

esp_err_t espnow_init(void) {
  s_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_event_t));
  if (s_espnow_queue == NULL) {
    LOGE(TAG, "Create mutex fail");
    return -1;
  }

  /* Initialize ESPNOW and register sending and receiving callback function. */
  esp_now_init();
  esp_now_register_send_cb(espnow_send_cb);
  esp_now_register_recv_cb(espnow_recv_cb);

  /* Set primary master key. */
  esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK);

  /* Initialize sending parameters. */
  send_param = malloc(sizeof(espnow_send_param_t));
  memset(send_param, 0, sizeof(espnow_send_param_t));
  if (send_param == NULL) {
    LOGE(TAG, "Malloc send parameter fail");
    vSemaphoreDelete(s_espnow_queue);
    esp_now_deinit();
    return -1;
  }
  send_param->unicast = true;
  send_param->broadcast = false;
  send_param->state = 0;
  send_param->magic = esp_random();
  send_param->count = CONFIG_ESPNOW_SEND_COUNT;
  send_param->delay = CONFIG_ESPNOW_SEND_DELAY;
  send_param->len = CONFIG_ESPNOW_SEND_LEN;
  send_param->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);
  if (send_param->buffer == NULL) {
    LOGE(TAG, "Malloc send buffer fail");
    free(send_param);
    vSemaphoreDelete(s_espnow_queue);
    esp_now_deinit();
    return -1;
  }

  // xTaskCreate(espnow_task, "espnow_task", 2048, send_param, 4, NULL);

  set_device_onboard(1);

  return ESP_OK;
}

void espnow_deinit(espnow_send_param_t *send_param) {
  if (send_param != NULL) {
    free(send_param->buffer);
    free(send_param);
  }
  vSemaphoreDelete(s_espnow_queue);
  esp_now_deinit();
}

unsigned int ascii_to_hex(const char *str, size_t size, uint8_t *hex) {
  unsigned int i, h, high, low;
  for (h = 0, i = 0; i < size; i += 2, ++h) {
    high = (str[i] > '9') ? str[i] - 'A' + 10 : str[i] - '0';
    low = (str[i + 1] > '9') ? str[i + 1] - 'A' + 10 : str[i + 1] - '0';
    hex[h] = (high << 4) | low;
  }
  return h;
}

int start_esp_now(void) {
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  uint8_t my_mac[6] = { 0 };
  char s_master_addr[16] = { 0 };

  esp_read_mac(my_mac, ESP_MAC_WIFI_SOFTAP);

  LOGI(TAG, "Mac addr : " MACSTR " ", MAC2STR(my_mac));

  sysevent_get(SYSEVENT_BASE, UNICAST_ADDR, &s_master_addr, sizeof(s_master_addr));

  ascii_to_hex(s_master_addr, sizeof(master_addr) * 2, master_addr);

  LOGI(TAG, "unicast send : %02X:%02X:%02X:%02X:%02X:%02X", master_addr[0], master_addr[1], master_addr[2],
       master_addr[3], master_addr[4], master_addr[5]);

  if (!espnow_task_handle) {
    xTaskCreatePinnedToCore(espnow_task, "espnow_task", 4096, send_param, task_priority, &espnow_task_handle, 0);
    if (!espnow_task_handle) {
      LOGE(TAG, "ESP-Now Task Start Failed!");
      return -1;
    }
  }
  return 0;
}

void send_msg_to_esp_now(uint8_t *data, uint32_t len) {
  espnow_data_prepare(send_param, data, len);
  set_data = 1;
}
