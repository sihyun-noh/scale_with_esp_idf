/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/projdefs.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lcd_panel_rgb_gt911.h"
// #include "config.h"
#include "mqtt.h"
#include "CAN_comn.h"
#include "ui/eez_agmo/src/ui/ui.h"
#include "ui_bridge.h"
#include "ui_adaptor.h"
#include "math_rpm.h"
#include "ui_msg.h"

static const char *TAG = "main_app";

bool lvgl_acquire(void);
void lvgl_release(void);
const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;
static SemaphoreHandle_t xGuiSemaphore = NULL;
static TaskHandle_t eez_lv_tick_task_handle;
static TaskHandle_t eez_lv_control_task_handle;

// 2026.01.14

typedef enum {
  CAN_ID_0X01 = 0x01,
  CAN_ID_0X02 = 0x02,
} can_id_t;

typedef enum {
  GUI_EVT_USER_DATA_INT,  // U1 - U9 표시
  GUI_EVT_VOLT_FLOAT      // 전압 float 표시
} gui_evt_type_t;

typedef struct {
  uint8_t rc_idx;  // 0:RC1, 1:RC2
  uint8_t motor;   // 1:M1, 2:M2
  float volt;
} gui_volt_evt_t;

typedef struct {
  gui_evt_type_t type;
  uint8_t can_id;
  uint8_t id;
  int32_t i_val;
  float f_val;
} gui_evt_t;

static QueueHandle_t s_gui_queue;
static QueueHandle_t s_tx_q;

typedef enum {
  TX_CMD_SET_DRIVER,      // 드롭다운으로 드라이버 선택
  TX_CMD_SET_USER_VALUE,  // 목표 전압 설정
  TX_CMD_SET_PERIOD,      // 주기 설정
  TX_CMD_RUN_STOP,        // RUN / STOP 버튼
} tx_cmd_type_t;

typedef struct {
  tx_cmd_type_t type;
  int32_t value;  // index, period ms, etc
  float fvalue;   // volt 같은 float 파라미터가 필요하면
  uint8_t payload[8];
} tx_cmd_t;

typedef enum {
  DRIVER_NONE = 0,
  DRIVER_RC1 = 1 << 0,  // 0x01
  DRIVER_RC2 = 1 << 1,  // 0x02
  DRIVER_BOTH = DRIVER_RC1 | DRIVER_RC2,
  DRIVER_EVT_BOTH
} driver_sel_t;

static driver_sel_t g_driver_sel = DRIVER_RC1;  // 기본값

static void build_cmd_run_rc1(uint8_t out8[8]) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = 0x04;
  out8[4] = 0x01;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
}

static void build_cmd_stop_rc1(uint8_t out8[8]) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = 0x04;
  out8[4] = 0x00;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
}

static void build_cmd_run_rc2(uint8_t out8[8]) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = 0x08;
  out8[4] = 0x01;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
}

static void build_cmd_stop_rc2(uint8_t out8[8]) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = 0x08;
  out8[4] = 0x00;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
}

static void build_cmd_both(uint8_t out8[8], bool run) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = 0x0b;
  if (run)
    out8[4] = 0x01;
  else
    out8[4] = 0x00;
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
}

bool tx_cmd_send(const tx_cmd_t *cmd) {
  if (!s_tx_q)
    return false;
  return xQueueSend(s_tx_q, cmd, 0) == pdTRUE;  // 논블로킹
}

bool lvgl_acquire(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (eez_lv_tick_task_handle != task) {
    return (xSemaphoreTake(xGuiSemaphore, 1000) == pdTRUE);
  }
  return false;
}

void lvgl_release(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (eez_lv_tick_task_handle != task) {
    xSemaphoreGive(xGuiSemaphore);
  }
}

static bool extract_uint_from_str(const char *s, uint32_t *out) {
  if (!s || !out)
    return false;

  uint32_t val = 0;
  bool any = false;

  while (*s) {
    if (isdigit((unsigned char)*s)) {
      any = true;
      val = val * 10u + (uint32_t)(*s - '0');
    }
    s++;
  }

  if (!any)
    return false;
  *out = val;
  return true;
}

// 전압 설정: 18 38 00 01 VV 00 00 00
static bool build_cmd_target_volt_from_str(driver_sel_t driver, const char *str, uint8_t out8[8]) {
  uint32_t v = 0;

  if (!extract_uint_from_str(str, &v))
    return false;
  if (v < 0 || v > 12)
    return false;

  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  if (driver == DRIVER_RC2)
    out8[3] = 0x05;  // user_data id
  else
    out8[3] = 0x01;      // user_data id
  out8[4] = (uint8_t)v;  // 1~12
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
  return true;
}

// 주기 설정: 18 38 00 03 PPlo Pphi 00 00  (PP = ms, little-endian)
static bool build_cmd_period_ms_from_str(driver_sel_t driver, const char *str, uint8_t out8[8]) {
  uint32_t ms = 0;
  if (!extract_uint_from_str(str, &ms))
    return false;
  if (ms < 100 || ms > 900)
    return false;

  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  if (driver == DRIVER_RC2)
    out8[3] = 0x07;  // user_data id
  else
    out8[3] = 0x03;                       // user_data id
  out8[4] = (uint8_t)(ms & 0xFF);         // low
  out8[5] = (uint8_t)((ms >> 8) & 0xFF);  // high
  out8[6] = 0x00;
  out8[7] = 0x00;
  return true;
}

// 전압 설정: 18 38 00 01 VV 00 00 00
static bool build_cmd_target_volt_from_int(driver_sel_t driver, uint32_t val, uint8_t out8[8]) {
  uint32_t v = val;

  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  if (driver == DRIVER_RC1)
    out8[3] = 0x01;  // user_data id
  else if (driver == DRIVER_RC2)
    out8[3] = 0x05;  // user_data id
  else if (driver == DRIVER_BOTH)
    out8[3] = 0x0c;  // user_data id
  else
    out8[3] = 0x0d;      // user_data id
  out8[4] = (uint8_t)v;  // 1~12
  out8[5] = 0x00;
  out8[6] = 0x00;
  out8[7] = 0x00;
  return true;
}

static bool build_cmd_target_volt_from_float(driver_sel_t driver, float val, uint8_t out8[8]) {
  out8[0] = 0x18;
  out8[1] = 0x38;
  out8[2] = 0x00;
  out8[3] = (driver == DRIVER_RC2) ? 0x05 : 0x01;  // user_data id

  // IEEE754 float(4B) -> out8[4..7]
  memcpy(&out8[4], &val, sizeof(float));  // 리틀엔디안 그대로 들어감(ESP32 기준)

  return true;
}

// 2026.01.14

/* ---- 리틀엔디안 4바이트 -> u32 ---- */
static inline uint32_t le_u32(const uint8_t b[4]) {
  return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

/* ---- u32 비트패턴 -> float (IEEE754) ---- */
static inline float u32_to_f32(uint32_t u) {
  float f;
  memcpy(&f, &u, sizeof(f));  // strict-aliasing 안전
  return f;
}

static inline int rc_idx_from_identifier(uint32_t id) {
  if (id == 0x01)
    return 1;  // RC1
  if (id == 0x02)
    return 2;  // RC2
  return -1;
}

/* ---- 디버그용 ---- */
static void print_hex(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02X%s", buf[i], (i + 1 < len) ? " " : "");
  }
  printf("\n");
}
static void parse_rx_and_push_gui(const twai_message_t *m) {
  if (!m)
    return;
  if (m->rtr)
    return;  // RTR이면 payload 없음(보통)
  if (m->data_length_code < 8)
    return;

  int rc_can_id = rc_idx_from_identifier(m->identifier);
  if (rc_can_id < 0)
    return;

  //
  if (m->data[0] == 0x10)
    return;

  // === 프로토콜 판별 ===
  // 네 코드가 data[1]을 봤는데, 실제 프로토콜에 맞춰 여기서 확정해야 함.
  // 여기서는 "data[1] 기준"을 그대로 유지하되, 필요하면 data[0]으로 바꾸면 됨.
  const uint8_t type = m->data[1];

  // 공통: ID는 data[3], 값은 data[4..7] (LE)
  const uint8_t id = m->data[3];
  const uint32_t raw = le_u32(&m->data[4]);

  gui_evt_t evt;
  memset(&evt, 0x00, sizeof(evt));

  if (type == 0x38) {
    // U1~U9 정수
    if (id >= 1 && id <= 10) {
      evt.type = GUI_EVT_USER_DATA_INT;
      evt.id = id;
      evt.f_val = u32_to_f32(raw);
      evt.i_val = (int32_t)raw;
      (void)xQueueSend(s_gui_queue, &evt, 0);
    }
    return;
  }

  if (type == 0x72) {
    // 전압 float (IEEE754)
    if (id >= 1 && id <= 2) {
      evt.type = GUI_EVT_VOLT_FLOAT;
      evt.id = id;  // motor id
      evt.f_val = u32_to_f32(raw);
      evt.can_id = (uint8_t)rc_can_id;
      // printf("can_id: %d, id: %d, val:%.2f\n", evt.can_id, evt.id, evt.f_val);
      (void)xQueueSend(s_gui_queue, &evt, 0);
    }
    return;
  }
}

// lvgl ui task base code
void lv_tick_task(void *arg) {
  while (1) {
    // raise the task priority of LVGL and/or reduce the handler period can improve the performance
    vTaskDelay(pdMS_TO_TICKS(5));
    // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_task_handler();
      // lv_timer_handler_run_in_period(5); /* run lv_timer_handler() every 5ms */
      xSemaphoreGive(xGuiSemaphore);
    }
  }
}
void eez_lv_hw_ctrl_task(void *arg) {
  static const bridge_t *queue = bridge_get();

  tx_cmd_t cmd;
  memset(&cmd, 0x00, sizeof(cmd));

  float target_volt = 0.0f;

  while (1) {
    ui_msg_t msg;
    if (xQueueReceive(queue->hw_q, &msg, portMAX_DELAY)) {
      memset(&cmd, 0x00, sizeof(cmd));

      switch (msg.id) {
        case UI_CMD_DRIVER1_SET:
          // TODO: 1 ~ 30 RPM의 값을 어떻게voltage값으로 변환할건지..
          // 1 -> ?? RPM으로 변화할건지..
          ESP_LOGI(TAG, "[Driver 1] ui_msg val %" PRId32, msg.value);

          // cmd send
          if (build_cmd_target_volt_from_int(DRIVER_RC1, msg.value, cmd.payload)) {
            // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
            cmd.type = TX_CMD_SET_USER_VALUE;
            tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_DRIVER2_SET:

          ESP_LOGI(TAG, "[Driver 2] ui_msg val %" PRId32, msg.value);
          // cmd send
          if (build_cmd_target_volt_from_int(DRIVER_RC2, msg.value, cmd.payload)) {
            // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
            cmd.type = TX_CMD_SET_USER_VALUE;
            tx_cmd_send(&cmd);
          }

          // motor2_set(msg.value);
          break;

        case UI_CMD_BOTH_SET_RPM:
          ESP_LOGI(TAG, "[Both Driver] ui_msg val %" PRId32, msg.value);
          if (build_cmd_target_volt_from_int(DRIVER_BOTH, msg.value, cmd.payload)) {
            // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
            cmd.type = TX_CMD_SET_USER_VALUE;
            tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_EVT_BOTH:
          ESP_LOGI(TAG, "[Evt both ] ui_msg val %s", msg.value ? "true" : "false");
          if (build_cmd_target_volt_from_int(DRIVER_EVT_BOTH, msg.value, cmd.payload)) {
            // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
            cmd.type = TX_CMD_SET_USER_VALUE;
            tx_cmd_send(&cmd);
          }
          break;

        case UI_CMD_RUN_STATE:
          ESP_LOGI(TAG, "[Run both] ui_msg val %s", msg.value ? "true" : "false");
          // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
          if (msg.value) {
            build_cmd_both(cmd.payload, true);
            cmd.type = TX_CMD_RUN_STOP;
            tx_cmd_send(&cmd);

          } else {
            build_cmd_both(cmd.payload, false);
            cmd.type = TX_CMD_RUN_STOP;
            tx_cmd_send(&cmd);
          }

          break;

        default: break;
      }
    }
  }
}

void eez_lv_tick_task(void *arg) {
  ui_init();
  static const bridge_t *queue = bridge_get();

  ESP_LOGI(TAG, "eez start");
  while (1) {
    // raise the task priority of LVGL and/or reduce the handler period can improve the performance
    vTaskDelay(pdMS_TO_TICKS(5));
    ui_tick();
    // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_task_handler();
      // lv_timer_handler_run_in_period(5); /* run lv_timer_handler() every 5ms */
      xSemaphoreGive(xGuiSemaphore);
    }

    ui_msg_t msg;
    // hw -> ui value set
    while (xQueueReceive(queue->ui_q, &msg, 0) == pdTRUE) {
      switch (msg.id) {
        case UI_EVT_FB_DRIVER1_R: set_var_fb_driver1_r(msg.str); break;

        case UI_EVT_FB_DRIVER1_L: set_var_fb_driver1_l(msg.str); break;

        case UI_EVT_FB_DRIVER2_R: set_var_fb_driver2_r(msg.str); break;

        case UI_EVT_FB_DRIVER2_L: set_var_fb_driver2_l(msg.str); break;

        default: break;
      }
    }
  }
}

static void twai_rx_task(void *arg) {
  (void)arg;
  twai_message_t msg;
  ESP_LOGI(TAG, "twai rx start!");
  while (1) {
    esp_err_t err = waveshare_twai_receive(&msg);  // 실시간 대기
    if (err == ESP_OK) {
#if 0
      // 필요하면 디버그 출력 (너무 많이 찍으면 지연됨)
      ESP_LOGI(TAG, "ID:%lx DLC:%d\n", msg.identifier, msg.data_length_code);
      if (!(msg.rtr)) {                                   // Check if it's not a Remote Transmission Request (RTR)
        for (int i = 0; i < msg.data_length_code; i++) {  // Loop through message data
          ESP_LOGI(TAG, " %d = %02x,", i, msg.data[i]);   // Print each byte in hex format
        }
        ESP_LOGI(TAG, "\r\n");  // New line after printing data
      }
#endif
      // ui task에 전달
      parse_rx_and_push_gui(&msg);

    } else if (err == ESP_ERR_TIMEOUT) {
      // 1초에 한 번만 찍히게 해서 로그 폭주 방지
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
#if 0
static void twai_tx_task(void *arg) {
  (void)arg;

  const TickType_t cmd_interval = pdMS_TO_TICKS(50);
  const TickType_t cycle_delay = pdMS_TO_TICKS(500);  // 예시: 한 싸이클 끝나고 쉬는 시간

  while (1) {
    for (int i = 0; i < 11; i++) {
      //      esp_err_t err = send_message_by_index(i);
      esp_err_t err = waveshare_twai_transmit(i);
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "TX fail idx=%d err=%d", i, (int)err);
      }
      vTaskDelay(cmd_interval);
    }
    vTaskDelay(cycle_delay);
  }
}
#endif

static void twai_tx_task(void *arg) {
  (void)arg;

  ESP_LOGI(TAG, "twai tx start!");
  const TickType_t cmd_interval = pdMS_TO_TICKS(50);
  const TickType_t cycle_delay = pdMS_TO_TICKS(50);

  tx_cmd_t cmd;

  while (1) {
    // 1) 먼저 "TX 명령"이 있는지 체크 (있으면 즉시 처리)
    //    0 tick으로 폴링하거나, 짧게 기다려도 됨.
    while (s_tx_q && xQueueReceive(s_tx_q, &cmd, 0) == pdTRUE) {
      switch (cmd.type) {
        case TX_CMD_SET_DRIVER:
          // ★ 여기서 기존 waveshare_twai_transmit(i) 말고 원하는 함수 실행
          // 예: waveshare_send_set_driver(cmd.value);
          //          waveshare_send_set_driver(cmd.value);
          break;

        case TX_CMD_SET_USER_VALUE: evt_twai_transmit(cmd.payload, CAN_ID_0X01); break;
        case TX_CMD_SET_PERIOD: evt_twai_transmit(cmd.payload, CAN_ID_0X01); break;
        case TX_CMD_RUN_STOP: evt_twai_transmit(cmd.payload, CAN_ID_0X01); break;

        default: break;
      }
    }
#if 1
    // 2) 이벤트가 없으면 기존 주기 송신 수행
    for (int i = 0; i < 2; i++) {
      ESP_LOGW(TAG, "TX  period");
      // 주기 송신 도중에도 이벤트가 들어오면 “빨리 반응”하고 싶으면
      // 매 전송 전/후로 큐를 한 번 더 확인하는 방식이 좋음.
      if (s_tx_q && xQueueReceive(s_tx_q, &cmd, 0) == pdTRUE) {
        // 받은 즉시 처리하고 i 루프는 계속하거나, break해서 다음 싸이클로 가도 됨
        // 여기서는 즉시 처리 후 계속 진행
        switch (cmd.type) {
            //  case TX_CMD_SET_DRIVER: waveshare_send_set_driver(cmd.value); break;
          case TX_CMD_SET_USER_VALUE: evt_twai_transmit(cmd.payload, CAN_ID_0X01); break;
          case TX_CMD_SET_PERIOD: evt_twai_transmit(cmd.payload, CAN_ID_0X01); break;
          case TX_CMD_RUN_STOP: evt_twai_transmit(cmd.payload, CAN_ID_0X01); break;
          default: break;
        }
      }

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
#endif
    vTaskDelay(cycle_delay);
  }
}

static void gui_update_task(void *arg) {
  (void)arg;

  gui_evt_t evt;
  ui_msg_id_t ui_evt = UI_CMD_NONE;

  char buf[32] = { 0 };
  while (1) {
    if (xQueueReceive(s_gui_queue, &evt, portMAX_DELAY) == pdTRUE) {
      switch (evt.type) {
        case GUI_EVT_USER_DATA_INT:
          // if (evt.id >= 1 && evt.id <= 10) {
          //   ESP_LOGI(TAG, "raw data : %.2f", evt.f_val);
          //   snprintf(buf, sizeof(buf), "%+.2f", (double)evt.f_val);
          //   bridge_send_to_ui(UI_EVT_FB_DRIVER1_R, buf, 0);
          // }
          break;

        case GUI_EVT_VOLT_FLOAT:
          // evt.id motor 1 ---> 1
          // evt.id motor 2 ---> 2
          // evt.can_id 0x01 ---> 1
          // evt.can_id 0x02 ---> 2
          //

          if (evt.can_id == 1) {
            if (evt.id == 1)
              ui_evt = UI_EVT_FB_DRIVER1_R;
            else if (evt.id == 2)
              ui_evt = UI_EVT_FB_DRIVER1_L;
          } else if (evt.can_id == 2) {
            if (evt.id == 1)
              ui_evt = UI_EVT_FB_DRIVER2_R;
            else if (evt.id == 2)
              ui_evt = UI_EVT_FB_DRIVER2_L;
          }

          if (ui_evt != UI_CMD_NONE) {
            snprintf(buf, sizeof(buf), "%+.2f", (double)evt.f_val);
            bridge_send_to_ui(ui_evt, buf, 0);
          }

          break;

        default: break;
      }
    }
  }
}

extern "C" {
void app_main(void) {
  xGuiSemaphore = xSemaphoreCreateMutex();

  ESP_LOGI(TAG, "main_...");
  ESP_LOGI(TAG, "Initialize LVGL library");
  lv_init();
  lv_port_disp_init();
  lv_port_12c_tp_init();
  lv_port_indev_init();
  lv_port_tick_init();
  ESP_LOGI(TAG, "init done");

  QueueHandle_t ui_q = xQueueCreate(32, sizeof(ui_msg_t));
  QueueHandle_t hw_q = xQueueCreate(32, sizeof(ui_msg_t));

  bridge_init_singleton(ui_q, hw_q);

  // 이후 UI task, HW task 생성

  /* clang-format off */

  // Create the EEZ UI tick task pinned to core 0.
//
// This task drives the EEZ-generated UI runtime on top of LVGL.
// Responsibilities:
// - Calls eez_ui_init() once to initialize the EEZ UI (create initial screens,
//   bind native variables, setup flows/actions).
// - Periodically calls eez_ui_tick() to advance EEZ flows/actions/timers and
//   propagate UI logic changes.
// - Typically runs alongside lv_timer_handler() (either inside this task or in
//   a separate LVGL task). If LVGL is not pumped elsewhere, this task should
//   also call lv_timer_handler() to keep rendering/input/animations alive.
//
// Note: Keep all EEZ/LVGL interactions in this single task (or protect with a mutex)
// to avoid multi-thread access to LVGL objects.
xTaskCreatePinnedToCore(
    eez_lv_tick_task,
    "eez_lv_tick_task",
    8192,
    NULL,
    5,
    &eez_lv_tick_task_handle,
    1
);


  // Create the LVGL/EEZ UI control task pinned to core 0.
  //
  // This task is the *single owner* of all UI state and LVGL/EEZ calls.
  // - Runs lv_timer_handler() and eez_ui_tick() periodically.
  // - Receives HW->UI events from the UI queue (bridge_send_evt) and applies them
  //   to EEZ native variables / UI widgets.
  // - Produces UI->HW commands when user changes EEZ variables (adapter -> bridge_send_cmd),
  //   which are delivered to the HW task via the HW queue.
  //
  // By centralizing UI updates and message dispatch in one task, we avoid
  // multi-thread access to LVGL (thread-safety) and keep UI<->HW communication deterministic.
  xTaskCreatePinnedToCore(
    eez_lv_hw_ctrl_task,
    "eez_lv_control_task",
    8192,
    NULL,
    5,
    &eez_lv_control_task_handle,
    1
);

  /* clang-format on */

#if 1
  vTaskDelay(pdMS_TO_TICKS(100));
  ESP_ERROR_CHECK(waveshare_twai_init());  // Initialize the TWAI interface and check for errors

  // lvgl msg queue init
  s_gui_queue = xQueueCreate(32, sizeof(gui_evt_t));
  if (!s_gui_queue) {
    ESP_LOGE(TAG, "Failed to create GUI queue");
    return;
  }
  // can tx msg queue init
  s_tx_q = xQueueCreate(16, sizeof(tx_cmd_t));
  if (!s_tx_q) {
    ESP_LOGE(TAG, "Failed to create tx msg queue");
    return;
  }

  xTaskCreatePinnedToCore(twai_rx_task, "twai_rx", 4096, NULL, 12, NULL, 0);
  xTaskCreatePinnedToCore(twai_tx_task, "twai_tx", 4096, NULL, 10, NULL, 0);

  // GUI/LVGL 태스크는 보통 core1에 두는 경우가 많음(프로젝트 정책에 따라)
  xTaskCreatePinnedToCore(gui_update_task, "gui_upd", 4096, NULL, 8, NULL, 1);

#endif
  while (1) {
    vTaskDelay(xDelay);
  }
}
}
