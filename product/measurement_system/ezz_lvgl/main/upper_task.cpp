#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "CAN_comn.h"
#include "monitor_runtime.h"
#include "upper_task.h"

static const char *TAG = "upper_task";

static constexpr uint32_t CANID_ESP_TO_VCU_CMD_RPM_TX = 0x18FF0200UL;
static constexpr uint32_t CANID_ESP_TO_VCU_CMD_TX = 0x18FF0210UL;
static constexpr uint32_t CANID_VCU_TO_ESP_STATUS_RPM_RX = 0x18FF0300UL;
static constexpr uint32_t CANID_VCU_TO_ESP_STATUS_RX = 0x18FF0310UL;

typedef struct {
  bool valid;
  uint8_t control_src;
  uint8_t stop_reason;
  uint8_t rc_status_mask;
  uint8_t vcu_fsm_status_mask;
  int16_t power_supply_value;
  uint8_t md_left_fault_msg;
  uint8_t md_right_fault_msg;
  uint8_t relay_st;
} upper_rx_status_t;

typedef struct {
  bool valid;
  int16_t left_axis1_rpm;
  int16_t left_axis2_rpm;
  int16_t right_axis1_rpm;
  int16_t right_axis2_rpm;
} upper_rx_status_rpm_t;

typedef struct {
  uint8_t driver_config_bitmask;
  bool cultivator_down;
  bool cultivator_on;
  bool upper_force_stop;
  bool upper_force_active;
  uint8_t relay_mask;
  bool automation;
  uint8_t reserved;
} upper_tx_cmd_t;

typedef struct {
  int16_t left_axis1_rpm;
  int16_t left_axis2_rpm;
  int16_t right_axis1_rpm;
  int16_t right_axis2_rpm;
} upper_tx_cmd_rpm_t;

static upper_rx_status_t s_upper_rx_status = {};
static upper_rx_status_rpm_t s_upper_rx_status_rpm = {};
static upper_tx_cmd_t s_upper_tx_cmd = {};
static upper_tx_cmd_rpm_t s_upper_tx_cmd_rpm = {};

static const char *control_src_name(uint8_t src) {
  switch (src) {
    case 0:
      return "NONE";
    case 1:
      return "RC";
    case 2:
      return "UPPER";
    default:
      return "UNKNOWN";
  }
}

static const char *stop_reason_name(uint8_t reason) {
  switch (reason) {
    case 0:
      return "NONE";
    case 1:
      return "UPPER_FORCE";
    case 2:
      return "RC_EMG";
    case 3:
      return "MOTOR_FAULT";
    case 4:
      return "TIMEOUT";
    default:
      return "UNKNOWN";
  }
}

static void format_rc_status_mask(uint8_t mask, char *buf, size_t len) {
  snprintf(buf, len, "RC:%c%c%c%c%c%c",
           (mask & (1u << 0)) ? 'E' : '-',
           (mask & (1u << 1)) ? 'S' : '-',
           (mask & (1u << 2)) ? 'F' : '-',
           (mask & (1u << 3)) ? 'R' : '-',
           (mask & (1u << 4)) ? 'D' : '-',
           (mask & (1u << 5)) ? 'O' : '-');
}

static void format_vcu_fsm_status_mask(uint8_t mask, char *buf, size_t len) {
  snprintf(buf, len, "FSM:%c%c%c%c%c%c%c%c",
           (mask & (1u << 0)) ? 'N' : '-',
           (mask & (1u << 1)) ? 'R' : '-',
           (mask & (1u << 2)) ? 'U' : '-',
           (mask & (1u << 3)) ? 'P' : '-',
           (mask & (1u << 4)) ? 'E' : '-',
           (mask & (1u << 5)) ? 'M' : '-',
           (mask & (1u << 6)) ? 'T' : '-',
           (mask & (1u << 7)) ? 'G' : '-');
}

static void print_hex(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02X%s", buf[i], (i + 1 < len) ? " " : "");
  }
  printf("\n");
}

static void publish_upper_status_rpm_feedback(const upper_rx_status_rpm_t *rpm) {
  char buf[32];

  snprintf(buf, sizeof(buf), "%d", (int)rpm->left_axis1_rpm);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS_RPM, UI_EVT_FB_DRIVER1_R, buf);

  snprintf(buf, sizeof(buf), "%d", (int)rpm->left_axis2_rpm);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS_RPM, UI_EVT_FB_DRIVER1_L, buf);

  snprintf(buf, sizeof(buf), "%d", (int)rpm->right_axis1_rpm);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS_RPM, UI_EVT_FB_DRIVER2_R, buf);

  snprintf(buf, sizeof(buf), "%d", (int)rpm->right_axis2_rpm);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS_RPM, UI_EVT_FB_DRIVER2_L, buf);
}

static void publish_upper_status_fields(const upper_rx_status_t *status) {
  char buf[32];

  snprintf(buf, sizeof(buf), "src:%s", control_src_name(status->control_src));
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D0, buf);

  snprintf(buf, sizeof(buf), "stop:%s", stop_reason_name(status->stop_reason));
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D1, buf);

  format_rc_status_mask(status->rc_status_mask, buf, sizeof(buf));
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D2, buf);

  format_vcu_fsm_status_mask(status->vcu_fsm_status_mask, buf, sizeof(buf));
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D3, buf);

  snprintf(buf, sizeof(buf), "pwr:%d", (int)status->power_supply_value);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D4, buf);

  snprintf(buf, sizeof(buf), "fltL:0x%02X", status->md_left_fault_msg);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D5, buf);

  snprintf(buf, sizeof(buf), "fltR:0x%02X", status->md_right_fault_msg);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D6, buf);

  snprintf(buf, sizeof(buf), "relay:0x%02X", status->relay_st);
  (void)monitor_runtime_post_text(GUI_EVT_VCU_STATUS, UI_EVT_VCU_STATUS_D7, buf);
}

static int16_t be16_to_i16(const uint8_t hi, const uint8_t lo) {
  return (int16_t)(((uint16_t)hi << 8) | (uint16_t)lo);
}

static void i16_to_be16(int16_t value, uint8_t *hi, uint8_t *lo) {
  uint16_t raw = (uint16_t)value;
  *hi = (uint8_t)((raw >> 8) & 0xFF);
  *lo = (uint8_t)(raw & 0xFF);
}

static bool upper_decode_rx_status(const twai_message_t *msg, upper_rx_status_t *out) {
  if (!msg || !out || !msg->extd || msg->identifier != CANID_VCU_TO_ESP_STATUS_RX || msg->data_length_code < 8) {
    return false;
  }

  memset(out, 0, sizeof(*out));
  out->valid = true;
  out->power_supply_value = be16_to_i16(msg->data[0], msg->data[1]);
  out->md_left_fault_msg = msg->data[2];
  out->md_right_fault_msg = msg->data[3];
  out->rc_status_mask = msg->data[4];
  out->vcu_fsm_status_mask = msg->data[5];
  out->relay_st = msg->data[6];
  out->control_src = 0;
  if (out->vcu_fsm_status_mask & (1u << 2)) {
    out->control_src = 2;
  } else if (out->vcu_fsm_status_mask & (1u << 1)) {
    out->control_src = 1;
  }
  if (out->vcu_fsm_status_mask & (1u << 3)) {
    out->stop_reason = 1;
  } else if (out->vcu_fsm_status_mask & (1u << 4)) {
    out->stop_reason = 2;
  } else if (out->vcu_fsm_status_mask & (1u << 5)) {
    out->stop_reason = 3;
  } else if (out->vcu_fsm_status_mask & (1u << 6)) {
    out->stop_reason = 4;
  }
  return true;
}

static bool upper_decode_rx_status_rpm(const twai_message_t *msg, upper_rx_status_rpm_t *out) {
  if (!msg || !out || !msg->extd || msg->identifier != CANID_VCU_TO_ESP_STATUS_RPM_RX || msg->data_length_code < 8) {
    return false;
  }

  memset(out, 0, sizeof(*out));
  out->valid = true;
  out->left_axis1_rpm = be16_to_i16(msg->data[0], msg->data[1]);
  out->left_axis2_rpm = be16_to_i16(msg->data[2], msg->data[3]);
  out->right_axis1_rpm = be16_to_i16(msg->data[4], msg->data[5]);
  out->right_axis2_rpm = be16_to_i16(msg->data[6], msg->data[7]);
  return true;
}

static void upper_pack_tx_cmd(const upper_tx_cmd_t *cmd, uint8_t out[8]) {
  memset(out, 0, 8);
  out[0] = cmd->driver_config_bitmask;
  out[1] = cmd->cultivator_down ? 1u : 0u;
  out[2] = cmd->cultivator_on ? 1u : 0u;
  out[3] = cmd->upper_force_stop ? 1u : 0u;
  out[4] = cmd->upper_force_active ? 1u : 0u;
  out[5] = cmd->relay_mask;
  out[6] = cmd->automation ? 1u : 0u;
  out[7] = cmd->reserved;
}

static void upper_pack_tx_cmd_rpm(const upper_tx_cmd_rpm_t *cmd, uint8_t out[8]) {
  i16_to_be16(cmd->left_axis1_rpm, &out[0], &out[1]);
  i16_to_be16(cmd->left_axis2_rpm, &out[2], &out[3]);
  i16_to_be16(cmd->right_axis1_rpm, &out[4], &out[5]);
  i16_to_be16(cmd->right_axis2_rpm, &out[6], &out[7]);
}

static void upper_rx_task(void *arg) {
  (void)arg;

  twai_message_t msg = {};
  ESP_LOGI(TAG, "upper rx start");

  while (1) {
    esp_err_t err = waveshare_twai_receive(&msg);
    if (err != ESP_OK) {
      if (err != ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "upper rx err=%s", esp_err_to_name(err));
      }
      continue;
    }

    if (upper_decode_rx_status(&msg, &s_upper_rx_status)) {
      ESP_LOGI(TAG, "upper status rx id=0x%08" PRIx32, msg.identifier);
      publish_upper_status_fields(&s_upper_rx_status);
      continue;
    }

    if (upper_decode_rx_status_rpm(&msg, &s_upper_rx_status_rpm)) {
      ESP_LOGI(TAG, "upper status rpm rx id=0x%08" PRIx32, msg.identifier);
      publish_upper_status_rpm_feedback(&s_upper_rx_status_rpm);
    }
  }
}

static void twai_rx_ext_test_task(void *arg) {
  (void)arg;

  twai_message_t msg = {};

  while (1) {
    esp_err_t err = waveshare_twai_receive(&msg);
    if (err == ESP_OK) {
      if (!msg.extd) {
        continue;
      }

      ESP_LOGI(TAG, "EXT RX ID:0x%08" PRIx32 " DLC:%d", msg.identifier, msg.data_length_code);

      if (msg.identifier == CANID_VCU_TO_ESP_STATUS_RX) {
        ESP_LOGW(TAG, "EXTENDED STATUS matched: 0x%08" PRIx32, CANID_VCU_TO_ESP_STATUS_RX);
        if (!msg.rtr) {
          print_hex(msg.data, msg.data_length_code);
          if (upper_decode_rx_status(&msg, &s_upper_rx_status)) {
            publish_upper_status_fields(&s_upper_rx_status);
          }
        }
      }

      if (msg.identifier == CANID_VCU_TO_ESP_STATUS_RPM_RX) {
        ESP_LOGW(TAG, "EXTENDED STATUS RPM matched: 0x%08" PRIx32, CANID_VCU_TO_ESP_STATUS_RPM_RX);
        if (!msg.rtr) {
          print_hex(msg.data, msg.data_length_code);
          if (upper_decode_rx_status_rpm(&msg, &s_upper_rx_status_rpm)) {
            publish_upper_status_rpm_feedback(&s_upper_rx_status_rpm);
          }
        }
      }
    } else if (err != ESP_ERR_TIMEOUT) {
      ESP_LOGE(TAG, "ext test receive err=%s", esp_err_to_name(err));
    }
  }
}

static void upper_tx_task(void *arg) {
  (void)arg;

  uint8_t payload[8];
  ESP_LOGI(TAG, "upper tx start");

  while (1) {
    upper_pack_tx_cmd(&s_upper_tx_cmd, payload);
    if (evt_twai_transmit(payload, CANID_ESP_TO_VCU_CMD_TX) != ESP_OK) {
      ESP_LOGW(TAG, "upper cmd tx failed");
    }

    upper_pack_tx_cmd_rpm(&s_upper_tx_cmd_rpm, payload);
    if (evt_twai_transmit(payload, CANID_ESP_TO_VCU_CMD_RPM_TX) != ESP_OK) {
      ESP_LOGW(TAG, "upper cmd rpm tx failed");
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void upper_process_run(void) {
  monitor_runtime_start();

  memset(&s_upper_rx_status, 0, sizeof(s_upper_rx_status));
  memset(&s_upper_rx_status_rpm, 0, sizeof(s_upper_rx_status_rpm));
  memset(&s_upper_tx_cmd, 0, sizeof(s_upper_tx_cmd));
  memset(&s_upper_tx_cmd_rpm, 0, sizeof(s_upper_tx_cmd_rpm));

  xTaskCreatePinnedToCore(twai_rx_ext_test_task, "twai_rx_ext_test_task", 4096, NULL, 12, NULL, 0);
  xTaskCreatePinnedToCore(upper_tx_task, "upper_tx", 4096, NULL, 10, NULL, 0);
}
