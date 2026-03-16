#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "CAN_comn.h"
#include "monitor_runtime.h"
#include "sibi_monitor.h"

static const char *TAG = "sibi_monitor";

typedef enum {
  CAN_ID_0X01 = 0x01,
  CAN_ID_0X02 = 0x02,
} can_id_t;

static inline uint32_t le_u32(const uint8_t b[4]) {
  return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static inline float u32_to_f32(uint32_t u) {
  float f;
  memcpy(&f, &u, sizeof(f));
  return f;
}

static inline int rc_idx_from_identifier(uint32_t id) {
  if (id == 0x01) {
    return 1;
  }
  if (id == 0x02) {
    return 2;
  }
  return -1;
}

static void parse_rx_and_push_gui(const twai_message_t *m) {
  if (!m || m->rtr || m->data_length_code < 8) {
    return;
  }

  int rc_can_id = rc_idx_from_identifier(m->identifier);
  if (rc_can_id < 0) {
    return;
  }

  if (m->data[0] == 0x10) {
    return;
  }

  const uint8_t type = m->data[1];
  const uint8_t id = m->data[3];
  const uint32_t raw = le_u32(&m->data[4]);

  gui_evt_t evt = {};

  if (type == 0x38) {
    if (id >= 1 && id <= 10) {
      evt.type = GUI_EVT_SIBI_USER_DATA_INT;
      evt.id = id;
      evt.f_val = u32_to_f32(raw);
      evt.i_val = (int32_t)raw;
      (void)monitor_runtime_post_gui_event(&evt);
    }
    return;
  }

  if (type == 0x72) {
    if (id >= 1 && id <= 2) {
      evt.type = GUI_EVT_SIBI_VOLT_FLOAT;
      evt.id = id;
      evt.f_val = u32_to_f32(raw);
      evt.can_id = (uint8_t)rc_can_id;
      (void)monitor_runtime_post_gui_event(&evt);
    }
  }
}

static void twai_rx_task(void *arg) {
  (void)arg;
  twai_message_t msg;
  ESP_LOGI(TAG, "twai rx start!");

  while (1) {
    esp_err_t err = waveshare_twai_receive(&msg);
    if (err == ESP_OK) {
      parse_rx_and_push_gui(&msg);
    } else if (err == ESP_ERR_TIMEOUT) {
      twai_status_info_t st;
      twai_get_status_info(&st);
      ESP_LOGW("TWAI_RX", "timeout. state=%d rx_q=%" PRIu32 " missed=%" PRIu32 " overrun=%" PRIu32 " bus_err=%" PRIu32,
               (int)st.state, (uint32_t)st.msgs_to_rx, (uint32_t)st.rx_missed_count, (uint32_t)st.rx_overrun_count,
               (uint32_t)st.bus_error_count);
    } else {
      ESP_LOGE("TWAI_RX", "receive err=%s", esp_err_to_name(err));
    }
  }
}

static void twai_tx_task(void *arg) {
  (void)arg;

  ESP_LOGI(TAG, "twai tx start!");
  const TickType_t cmd_interval = pdMS_TO_TICKS(50);
  const TickType_t cycle_delay = pdMS_TO_TICKS(50);

  tx_cmd_t cmd;

  while (1) {
    while (monitor_runtime_tx_cmd_receive(&cmd, 0)) {
      switch (cmd.type) {
        case TX_CMD_SET_DRIVER:
          break;
        case TX_CMD_SET_USER_VALUE:
          evt_twai_transmit(cmd.payload, CAN_ID_0X01);
          break;
        case TX_CMD_SET_PERIOD:
          evt_twai_transmit(cmd.payload, CAN_ID_0X01);
          break;
        case TX_CMD_RUN_STOP:
          evt_twai_transmit(cmd.payload, CAN_ID_0X01);
          break;
        default:
          break;
      }
    }

    for (int i = 0; i < 2; i++) {
      ESP_LOGW(TAG, "TX period");

      esp_err_t err = waveshare_twai_transmit(i, CAN_ID_0X01);
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "TX fail idx=%d err=%d", i, (int)err);
      }

      err = waveshare_twai_transmit(i, CAN_ID_0X02);
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "TX fail idx=%d err=%d", i, (int)err);
      }

      vTaskDelay(cmd_interval);
    }
    vTaskDelay(cycle_delay);
  }
}

void sibi_process_run(void) {
  monitor_runtime_start();
  xTaskCreatePinnedToCore(twai_rx_task, "twai_rx", 4096, NULL, 12, NULL, 0);
  xTaskCreatePinnedToCore(twai_tx_task, "twai_tx", 4096, NULL, 10, NULL, 0);
}
