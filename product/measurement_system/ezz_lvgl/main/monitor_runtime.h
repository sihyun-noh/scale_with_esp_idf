#ifndef MONITOR_RUNTIME_H
#define MONITOR_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "ui_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  GUI_EVT_SIBI_USER_DATA_INT = 0,
  GUI_EVT_SIBI_VOLT_FLOAT,
  GUI_EVT_VCU_STATUS,
  GUI_EVT_VCU_STATUS_RPM,
  GUI_EVT_UI_TEXT,
} gui_evt_type_t;

typedef struct {
  gui_evt_type_t type;
  uint8_t can_id;
  uint8_t id;
  int32_t i_val;
  float f_val;
  ui_msg_id_t ui_evt;
  char text[32];
} gui_evt_t;

typedef enum {
  TX_CMD_SET_DRIVER = 0,
  TX_CMD_SET_USER_VALUE,
  TX_CMD_SET_PERIOD,
  TX_CMD_RUN_STOP,
  TX_CMD_3XIS_THROTTLE,
  TX_CMD_3XIS_STEERING,
  TX_CMD_3XIS_AUTOMATION,
} tx_cmd_type_t;

typedef struct {
  tx_cmd_type_t type;
  int32_t value;
  float fvalue;
  uint8_t payload[8];
} tx_cmd_t;

void monitor_runtime_start(void);
bool monitor_runtime_tx_cmd_send(const tx_cmd_t *cmd);
bool monitor_runtime_tx_cmd_receive(tx_cmd_t *cmd, TickType_t ticks_to_wait);
bool monitor_runtime_post_gui_event(const gui_evt_t *evt);
bool monitor_runtime_post_text(gui_evt_type_t type, ui_msg_id_t ui_evt, const char *text);

#ifdef __cplusplus
}
#endif

#endif
