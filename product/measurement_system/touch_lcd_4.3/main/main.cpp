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
#include "LCD_Wifi_Manager.h"
#include "ui/ui.h"
#include "ui/ui_events.h"
#include "mqtt.h"
#include "CAN_comn.h"
#include "widgets/lv_label.h"

static const char *TAG = "main_app";

extern "C" {
// extern void example_lvgl_demo_ui(lv_disp_t *disp);
//  void lv_example_anim_2(lv_disp_t *disp);
extern void ap_httpserver_start(void);
extern void dpp_enrollee_init_start(void);
extern void wifi_scan_main(void);
extern void wifi_scan_done(void);
extern void littlevgl_wificonfig_init(void);
extern void lv_example_list_1(void);
extern void wifi_scan_btn(void);
}

bool lvgl_acquire(void);
void lvgl_release(void);
const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;
static SemaphoreHandle_t xGuiSemaphore = NULL;
static TaskHandle_t g_lvgl_task_handle;

// 2026.01.14

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

typedef enum {
  TX_CMD_SET_DRIVER,       // 드롭다운으로 드라이버 선택
  TX_CMD_SET_TARGET_VOLT,  // 목표 전압 설정
  TX_CMD_SET_PERIOD,       // 주기 설정
  TX_CMD_RUN,              // RUN 버튼
  TX_CMD_STOP,             // STOP 버튼
} tx_cmd_type_t;

typedef struct {
  tx_cmd_type_t type;
  int32_t value;  // index, period ms, etc
  float fvalue;   // volt 같은 float 파라미터가 필요하면
  uint8_t payload[8];
} tx_cmd_t;

static QueueHandle_t s_tx_q;

typedef enum {
  DRIVER_NONE = 0,
  DRIVER_RC1 = 1 << 0,  // 0x01
  DRIVER_RC2 = 1 << 1,  // 0x02
  DRIVER_BOTH = DRIVER_RC1 | DRIVER_RC2
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

void tx_cmd_init(void) {
  s_tx_q = xQueueCreate(16, sizeof(tx_cmd_t));
}

bool tx_cmd_send(const tx_cmd_t *cmd) {
  if (!s_tx_q)
    return false;
  return xQueueSend(s_tx_q, cmd, 0) == pdTRUE;  // 논블로킹
}

void lv_example_qrcode_1(lv_disp_t *disp) {
  lv_obj_t *scr = lv_disp_get_scr_act(disp);

  // setStyle();
  // makeKeyboard();
  // buildStatusBar();
  // buildPWMsgBox();
  // buildBody();
  // buildSettings();

  // lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
  // lv_color_t fg_color = lv_palette_darken(LV_PALETTE_BLUE, 4);
  // lv_obj_t *qr = lv_qrcode_create(scr, 255, fg_color, bg_color);

  // /*Set data*/
  // const char *data = "http://192.168.4.1/hello";
  // lv_qrcode_update(qr, data, strlen(data));
  // lv_obj_center(qr);

  // /*Add a border with bg_color*/
  // lv_obj_set_style_border_color(qr, bg_color, 0);
  // lv_obj_set_style_border_width(qr, 5, 0);
}

bool lvgl_acquire(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (g_lvgl_task_handle != task) {
    return (xSemaphoreTake(xGuiSemaphore, 1000) == pdTRUE);
  }
  return false;
}

void lvgl_release(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (g_lvgl_task_handle != task) {
    xSemaphoreGive(xGuiSemaphore);
  }
}

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
    return 0;  // RC1
  if (id == 0x02)
    return 1;  // RC2
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
    if (id >= 1 && id <= 9) {
      evt.type = GUI_EVT_USER_DATA_INT;
      evt.id = id;
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
      (void)xQueueSend(s_gui_queue, &evt, 0);
    }
    return;
  }

  // 기타 타입이면 무시 또는 로그
}

static void twai_rx_task(void *arg) {
  (void)arg;
  twai_message_t msg;

  while (1) {
    esp_err_t err = waveshare_twai_receive(&msg);  // 실시간 대기
    if (err == ESP_OK) {
      // 필요하면 디버그 출력 (너무 많이 찍으면 지연됨)
      // printf("ID:%lx DLC:%d\n", msg.identifier, msg.data_length_code);
      parse_rx_and_push_gui(&msg);
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

  const TickType_t cmd_interval = pdMS_TO_TICKS(50);
  const TickType_t cycle_delay = pdMS_TO_TICKS(500);

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

        case TX_CMD_SET_TARGET_VOLT: evt_twai_transmit(cmd.payload); break;

        case TX_CMD_SET_PERIOD: evt_twai_transmit(cmd.payload); break;

        case TX_CMD_RUN: evt_twai_transmit(cmd.payload); break;

        case TX_CMD_STOP:
          // waveshare_send_stop();
          break;

        default: break;
      }
    }

    // 2) 이벤트가 없으면 기존 주기 송신 수행
    for (int i = 0; i < 11; i++) {
      // 주기 송신 도중에도 이벤트가 들어오면 “빨리 반응”하고 싶으면
      // 매 전송 전/후로 큐를 한 번 더 확인하는 방식이 좋음.
      if (s_tx_q && xQueueReceive(s_tx_q, &cmd, 0) == pdTRUE) {
        // 받은 즉시 처리하고 i 루프는 계속하거나, break해서 다음 싸이클로 가도 됨
        // 여기서는 즉시 처리 후 계속 진행
        switch (cmd.type) {
            //  case TX_CMD_SET_DRIVER: waveshare_send_set_driver(cmd.value); break;
          case TX_CMD_SET_TARGET_VOLT: evt_twai_transmit(cmd.payload); break;
          case TX_CMD_SET_PERIOD: evt_twai_transmit(cmd.payload); break;
          case TX_CMD_RUN: evt_twai_transmit(cmd.payload); break;
          default: break;
        }
      }

      esp_err_t err = waveshare_twai_transmit(i);
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "TX fail idx=%d err=%d", i, (int)err);
      }

      vTaskDelay(cmd_interval);
    }

    vTaskDelay(cycle_delay);
  }
}

static void gui_update_task(void *arg) {
  (void)arg;

  // 라벨 매핑 (id -> label)
  lv_obj_t *user_labels[10] = { NULL,        ui_Label_10, ui_Label_11, ui_Label_12, ui_Label_13,
                                ui_Label_14, ui_Label_15, ui_Label_16, ui_Label_17, ui_Label_18 };

  // rc_idx: 0=RC1, 1=RC2
  // m_idx : 1=M1, 2=M2 (0 unused)
  static lv_obj_t *volt_labels[2][3] = {
    { NULL, ui_Label_volt_RC1_M1, ui_Label_volt_RC1_M2 },
    { NULL, ui_Label_volt_RC2_M1, ui_Label_volt_RC2_M2 },
  };

  gui_evt_t evt;
  char buf[32];

  while (1) {
    if (xQueueReceive(s_gui_queue, &evt, portMAX_DELAY) == pdTRUE) {
      switch (evt.type) {
        case GUI_EVT_USER_DATA_INT:
          if (evt.id >= 1 && evt.id <= 9 && user_labels[evt.id]) {
            lv_label_set_text_fmt(user_labels[evt.id], "U%d:%ld", evt.id, (long)evt.i_val);
          }
          break;

        case GUI_EVT_VOLT_FLOAT:
          if (evt.can_id < 2 && evt.id >= 1 && evt.id <= 2) {
            // +12.00 같은 포맷
            lv_obj_t *lbl = volt_labels[evt.can_id][evt.id];
            snprintf(buf, sizeof(buf), "%+.2f", (double)evt.f_val);
            lv_label_set_text(lbl, buf);
          }
          break;

        default: break;
      }
    }
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

extern "C" {
void app_main(void) {
  // ap_httpserver_start();
  //  dpp_enrollee_init_start();

  xGuiSemaphore = xSemaphoreCreateMutex();

  ESP_LOGI(TAG, "main_...");
  ESP_LOGI(TAG, "Initialize LVGL library");
  lv_init();
  lv_port_disp_init();
  lv_port_12c_tp_init();
  lv_port_indev_init();
  lv_port_tick_init();
  ESP_LOGI(TAG, "init done");

  xTaskCreatePinnedToCore(lv_tick_task, "lv_tick_task", 8192, NULL, 1, &g_lvgl_task_handle, 0);

  wifi_scan_main();
  // example_lvgl_demo_ui(disp);
  // lv_example_anim_2(disp);
  // lv_example_qrcode_1(disp);
  // ui_init_3dprinter();
  ui_init();
  lvgl_acquire();
  wifi_scan_btn();
  lvgl_release();
  //  mqtt_app_start();

  lv_evt_init();

  ESP_ERROR_CHECK(waveshare_twai_init());  // Initialize the TWAI interface and check for errors

  // lvgl msg queue init
  s_gui_queue = xQueueCreate(32, sizeof(gui_evt_t));
  if (!s_gui_queue) {
    ESP_LOGE(TAG, "Failed to create GUI queue");
    return;
  }
  // can tx queue init
  tx_cmd_init();

  xTaskCreatePinnedToCore(twai_rx_task, "twai_rx", 4096, NULL, 12, NULL, 0);
  xTaskCreatePinnedToCore(twai_tx_task, "twai_tx", 4096, NULL, 10, NULL, 0);

  // GUI/LVGL 태스크는 보통 core1에 두는 경우가 많음(프로젝트 정책에 따라)
  xTaskCreatePinnedToCore(gui_update_task, "gui_upd", 4096, NULL, 8, NULL, 1);

  app_evt_t evt;

  while (1) {
    vTaskDelay(xDelay);
    if (lv_evt_recv(&evt, portMAX_DELAY)) {
      tx_cmd_t cmd;
      memset(&cmd, 0x00, sizeof(cmd));
      switch (evt.src) {
        case UI_SRC_DRIVER_DD:
          ESP_LOGI(TAG, "dropdown msg : %s", evt.str);  // start logic

          if (strncmp(evt.str, "RC1", 3) == 0) {
            g_driver_sel = DRIVER_RC1;
          } else if (strncmp(evt.str, "RC2", 3) == 0) {
            g_driver_sel = DRIVER_RC2;
          } else if (strncmp(evt.str, "Both", 4) == 0) {
            g_driver_sel = DRIVER_BOTH;
          }

          break;

        case UI_SRC_TARGET_VOLT:
          ESP_LOGI(TAG, "dropdown msg : %s", evt.str);  // start logic
          if (g_driver_sel == DRIVER_RC1) {
            if (build_cmd_target_volt_from_str(DRIVER_RC1, evt.str, cmd.payload)) {
              // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
              cmd.type = TX_CMD_SET_TARGET_VOLT;
              tx_cmd_send(&cmd);
            }
          } else if (g_driver_sel == DRIVER_RC2) {
            if (build_cmd_target_volt_from_str(DRIVER_RC2, evt.str, cmd.payload)) {
              // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
              cmd.type = TX_CMD_SET_TARGET_VOLT;
              tx_cmd_send(&cmd);
            }
          } else if (g_driver_sel == DRIVER_BOTH) {
            if (build_cmd_target_volt_from_str(DRIVER_RC1, evt.str, cmd.payload)) {
              // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
              cmd.type = TX_CMD_SET_TARGET_VOLT;
              tx_cmd_send(&cmd);
            }

            if (build_cmd_target_volt_from_str(DRIVER_RC2, evt.str, cmd.payload)) {
              // 예: 선택된 드라이버 CAN ID를 current_driver_id로 들고 있다 가정
              cmd.type = TX_CMD_SET_TARGET_VOLT;
              tx_cmd_send(&cmd);
            }
          }

          // stop logic
          break;

        case UI_SRC_MODE_PERIOD_SET:
          ESP_LOGI(TAG, "dropdown msg : %s", evt.str);  // start logic
          if (g_driver_sel == DRIVER_RC1) {
            if (build_cmd_period_ms_from_str(DRIVER_RC1, evt.str, cmd.payload)) {
              cmd.type = TX_CMD_SET_PERIOD;
              tx_cmd_send(&cmd);
            }
          } else if (g_driver_sel == DRIVER_RC2) {
            if (build_cmd_period_ms_from_str(DRIVER_RC2, evt.str, cmd.payload)) {
              cmd.type = TX_CMD_SET_PERIOD;
              tx_cmd_send(&cmd);
            }
          } else if (g_driver_sel == DRIVER_BOTH) {
            if (build_cmd_period_ms_from_str(DRIVER_RC1, evt.str, cmd.payload)) {
              cmd.type = TX_CMD_SET_PERIOD;
              tx_cmd_send(&cmd);
            }
            if (build_cmd_period_ms_from_str(DRIVER_RC2, evt.str, cmd.payload)) {
              cmd.type = TX_CMD_SET_PERIOD;
              tx_cmd_send(&cmd);
            }
          }

          // stop logic
          break;
        case UI_SRC_BTN_RUN:
          ESP_LOGI(TAG, "dropdown msg : %s", evt.str);  // start logic
          if (g_driver_sel == DRIVER_RC1) {
            if (strncmp(evt.str, "Run", 3) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_run_rc1(cmd.payload);
              tx_cmd_send(&cmd);
            } else if (strncmp(evt.str, "Stop", 4) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_stop_rc1(cmd.payload);
              tx_cmd_send(&cmd);
            }
          }

          if (g_driver_sel == DRIVER_RC2) {
            if (strncmp(evt.str, "Run", 3) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_run_rc2(cmd.payload);
              tx_cmd_send(&cmd);
            } else if (strncmp(evt.str, "Stop", 4) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_stop_rc2(cmd.payload);
              tx_cmd_send(&cmd);
            }
          }

          if (g_driver_sel == DRIVER_BOTH) {
            if (strncmp(evt.str, "Run", 3) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_run_rc1(cmd.payload);
              tx_cmd_send(&cmd);
            } else if (strncmp(evt.str, "Stop", 4) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_stop_rc1(cmd.payload);
              tx_cmd_send(&cmd);
            }

            if (strncmp(evt.str, "Run", 3) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_run_rc2(cmd.payload);
              tx_cmd_send(&cmd);
            } else if (strncmp(evt.str, "Stop", 4) == 0) {
              cmd.type = TX_CMD_RUN;
              build_cmd_stop_rc2(cmd.payload);
              tx_cmd_send(&cmd);
            }
          }

          // stop logic
          break;

        default: break;
      }
    }
  }
}
}
