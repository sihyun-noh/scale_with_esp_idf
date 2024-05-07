/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lcd_panel_rgb_gt911.h"
#include "config.h"
#include "LCD_Wifi_Manager.h"
#include "ui/ui.h"
#include "mqtt.h"

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

  while (1) {
    vTaskDelay(xDelay);
  }
}
}
