#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "monitor_runtime.h"
#include "ui_bridge.h"

static const char *TAG = "monitor_runtime";

typedef enum {
  CAN_ID_0X01 = 0x01,
} can_id_t;

typedef enum {
  DRIVER_NONE = 0,
  DRIVER_RC1 = 1 << 0,
  DRIVER_RC2 = 1 << 1,
  DRIVER_BOTH = DRIVER_RC1 | DRIVER_RC2,
  DRIVER_EVT_BOTH
} driver_sel_t;

static QueueHandle_t s_gui_queue;
static QueueHandle_t s_tx_q;
static bool s_runtime_started;
static driver_sel_t s_driver_sel = DRIVER_RC1;

static bool build_cmd_target_volt_from_int(driver_sel_t driver, uint32_t val, uint8_t out8[8]) {
  uint32_t v = val;

  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  if (driver == DRIVER_RC1) {
    out8[3] = 0x01;
  } else if (driver == DRIVER_RC2) {
    out8[3] = 0x05;
  } else if (driver == DRIVER_BOTH) {
    out8[3] = 0x0c;
  } else {
    out8[3] = 0x0d;
  }
  out8[4] = (uint8_t)v;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
  return true;
}

static void build_cmd_both(uint8_t out8[8], bool run) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = 0x0b;
  out8[4] = run ? 0x01 : 0x00;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
}

bool monitor_runtime_tx_cmd_send(const tx_cmd_t *cmd) {
  if (!s_tx_q || !cmd) {
    return false;
  }
  return xQueueSend(s_tx_q, cmd, 0) == pdTRUE;
}

bool monitor_runtime_tx_cmd_receive(tx_cmd_t *cmd, TickType_t ticks_to_wait) {
  if (!s_tx_q || !cmd) {
    return false;
  }
  return xQueueReceive(s_tx_q, cmd, ticks_to_wait) == pdTRUE;
}

bool monitor_runtime_post_gui_event(const gui_evt_t *evt) {
  if (!s_gui_queue) {
    return false;
  }
  return xQueueSend(s_gui_queue, evt, 0) == pdTRUE;
}

bool monitor_runtime_post_text(gui_evt_type_t type, ui_msg_id_t ui_evt, const char *text) {
  gui_evt_t evt = {};
  evt.type = type;
  evt.ui_evt = ui_evt;
  if (text) {
    snprintf(evt.text, sizeof(evt.text), "%s", text);
  }
  return monitor_runtime_post_gui_event(&evt);
}

static void eez_lv_hw_ctrl_task(void *arg) {
  (void)arg;
  static const bridge_t *queue = bridge_get();

  tx_cmd_t cmd = {};

  while (1) {
    ui_msg_t msg;
    if (xQueueReceive(queue->hw_q, &msg, portMAX_DELAY)) {
      memset(&cmd, 0x00, sizeof(cmd));

      switch (msg.id) {
        case UI_CMD_DRIVER1_SET:
          ESP_LOGI(TAG, "[Driver 1] ui_msg val %" PRId32, msg.value);
          if (build_cmd_target_volt_from_int(DRIVER_RC1, msg.value, cmd.payload)) {
            cmd.type = TX_CMD_SET_USER_VALUE;
            monitor_runtime_tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_DRIVER2_SET:
          ESP_LOGI(TAG, "[Driver 2] ui_msg val %" PRId32, msg.value);
          if (build_cmd_target_volt_from_int(DRIVER_RC2, msg.value, cmd.payload)) {
            cmd.type = TX_CMD_SET_USER_VALUE;
            monitor_runtime_tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_BOTH_SET_RPM:
          ESP_LOGI(TAG, "[Both Driver] ui_msg val %" PRId32, msg.value);
          if (build_cmd_target_volt_from_int(DRIVER_BOTH, msg.value, cmd.payload)) {
            cmd.type = TX_CMD_SET_USER_VALUE;
            monitor_runtime_tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_EVT_BOTH:
          ESP_LOGI(TAG, "[Evt both ] ui_msg val %s", msg.value ? "true" : "false");
          if (build_cmd_target_volt_from_int(DRIVER_EVT_BOTH, msg.value, cmd.payload)) {
            cmd.type = TX_CMD_SET_USER_VALUE;
            monitor_runtime_tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_RUN_STATE:
          ESP_LOGI(TAG, "[Run both] ui_msg val %s", msg.value ? "true" : "false");
          build_cmd_both(cmd.payload, msg.value);
          cmd.type = TX_CMD_RUN_STOP;
          monitor_runtime_tx_cmd_send(&cmd);
          break;

        default: break;
      }
    }
  }
}

static void gui_update_task(void *arg) {
  (void)arg;

  gui_evt_t evt;
  char buf[32] = { 0 };

  while (1) {
    if (xQueueReceive(s_gui_queue, &evt, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    switch (evt.type) {
      case GUI_EVT_SIBI_USER_DATA_INT: break;

      case GUI_EVT_SIBI_VOLT_FLOAT: {
        ui_msg_id_t ui_evt = UI_CMD_NONE;
        if (evt.can_id == 1) {
          if (evt.id == 1) {
            ui_evt = UI_EVT_FB_DRIVER1_R;
          } else if (evt.id == 2) {
            ui_evt = UI_EVT_FB_DRIVER1_L;
          }
        } else if (evt.can_id == 2) {
          if (evt.id == 1) {
            ui_evt = UI_EVT_FB_DRIVER2_R;
          } else if (evt.id == 2) {
            ui_evt = UI_EVT_FB_DRIVER2_L;
          }
        }

        if (ui_evt != UI_CMD_NONE) {
          snprintf(buf, sizeof(buf), "%+.2f", (double)evt.f_val);
          bridge_send_to_ui(ui_evt, buf, 0);
        }
        break;
      }

      case GUI_EVT_VCU_STATUS:
        if (evt.ui_evt != UI_CMD_NONE) {
          bridge_send_to_ui(evt.ui_evt, evt.text, 0);
        }
        break;

      case GUI_EVT_VCU_STATUS_RPM:
        if (evt.ui_evt != UI_CMD_NONE) {
          bridge_send_to_ui(evt.ui_evt, evt.text, 0);
        }
        break;

      case GUI_EVT_UI_TEXT:
        if (evt.ui_evt != UI_CMD_NONE) {
          bridge_send_to_ui(evt.ui_evt, evt.text, 0);
        }
        break;

      default: break;
    }
  }
}

void monitor_runtime_start(void) {
  if (s_runtime_started) {
    return;
  }

  s_gui_queue = xQueueCreate(32, sizeof(gui_evt_t));
  if (!s_gui_queue) {
    ESP_LOGE(TAG, "Failed to create GUI queue");
    return;
  }

  s_tx_q = xQueueCreate(16, sizeof(tx_cmd_t));
  if (!s_tx_q) {
    ESP_LOGE(TAG, "Failed to create tx msg queue");
    return;
  }

  (void)s_driver_sel;

  xTaskCreatePinnedToCore(eez_lv_hw_ctrl_task, "eez_lv_control_task", 8192, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(gui_update_task, "gui_upd", 4096, NULL, 8, NULL, 1);

  s_runtime_started = true;
}
