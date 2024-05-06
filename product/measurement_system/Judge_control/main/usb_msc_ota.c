/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_msc_host.h"
#include "esp_msc_ota.h"
#include "log.h"
#include "file_copy.h"
#include "usb/usb_host.h"

#include "sysevent.h"
#include "config.h"
#include "ui.h"

#define OTA_FILE_NAME "/usb/" CONFIG_MSC_OTA_BIN_NAME
static const char *TAG = "usb_msc_ota";
static TaskHandle_t usb_msc_ota_handle = NULL;

extern void create_custom_msg_box(const char *msg_text, lv_obj_t *active_screen, void (*event_handler)(lv_event_t *),
                                  lv_event_code_t event);

lv_obj_t *ui_msc_ota_panel;
lv_obj_t *ui_msc_ota_panel_top_Label;
lv_obj_t *ui_msc_ota_panel_bottom_Label;
bool update_flag = false;
static char s_buf[100] = { 0 };
static int ota_updata_percent = 0;

void ui_msc_ota_update() {
  update_flag = true;
  ui_msc_ota_panel = lv_obj_create(lv_scr_act());
  lv_obj_set_width(ui_msc_ota_panel, 300);
  lv_obj_set_height(ui_msc_ota_panel, 150);
  lv_obj_set_x(ui_msc_ota_panel, 0);
  lv_obj_set_y(ui_msc_ota_panel, 0);
  lv_obj_set_align(ui_msc_ota_panel, LV_ALIGN_CENTER);
  // lv_obj_add_flag(ui_msc_ota_panel, LV_OBJ_FLAG_HIDDEN);        /// Flags
  lv_obj_clear_flag(ui_msc_ota_panel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_msc_ota_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_msc_ota_panel, lv_color_hex(0xE8BABA), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_msc_ota_panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_msc_ota_panel_top_Label = lv_label_create(ui_msc_ota_panel);
  lv_obj_set_width(ui_msc_ota_panel_top_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_msc_ota_panel_top_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_msc_ota_panel_top_Label, 0);
  lv_obj_set_y(ui_msc_ota_panel_top_Label, 0);
  lv_obj_align(ui_msc_ota_panel_top_Label, LV_ALIGN_CENTER, -50, -20);
  lv_label_set_text(ui_msc_ota_panel_top_Label, "업데이트:");
  lv_obj_set_style_text_font(ui_msc_ota_panel_top_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_msc_ota_panel_bottom_Label = lv_label_create(ui_msc_ota_panel);
  lv_obj_set_width(ui_msc_ota_panel_bottom_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_msc_ota_panel_bottom_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_msc_ota_panel_bottom_Label, 0);
  lv_obj_set_y(ui_msc_ota_panel_bottom_Label, 0);
  lv_obj_align(ui_msc_ota_panel_bottom_Label, LV_ALIGN_CENTER, -50, 20);
  lv_label_set_text(ui_msc_ota_panel_bottom_Label, " ");
  lv_obj_set_style_text_font(ui_msc_ota_panel_bottom_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void print_progressbar(float progress, float total) {
  const int bar_width = 50;
  int filled_width = progress * bar_width / total;
  // char s_buf[10]={0};
  printf("%s[", LOG_COLOR_I);
  for (int i = 0; i < bar_width; ++i) {
    if (i < filled_width) {
      printf(">");
    } else {
      printf(" ");
    }
  }
  printf(" ]%s%d%%\r", LOG_RESET_COLOR, filled_width * 100 / bar_width);
  ota_updata_percent = (int)filled_width * 100 / bar_width;
  // sprintf(s_buf, "%d", filled_width * 100 / bar_width);
  //  sysevent_set(MSC_OTA_UPDATE, s_buf);
}

static void msc_ota_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  switch (event_id) {
    case ESP_MSC_OTA_START: ESP_LOGI(TAG, "ESP_MSC_OTA_START"); break;
    case ESP_MSC_OTA_READY_UPDATE:
      ESP_LOGI(TAG, "ESP_MSC_OTA_READY_UPDATE");
      ui_msc_ota_update();
      //_ui_flag_modify(ui_msc_ota_panel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
      break;
    case ESP_MSC_OTA_WRITE_FLASH:
      float progress = *(float *)event_data;
      print_progressbar(progress, 1.0);
      break;
    case ESP_MSC_OTA_FAILED: ESP_LOGI(TAG, "ESP_MSC_OTA_FAILED"); break;
    case ESP_MSC_OTA_GET_IMG_DESC: ESP_LOGI(TAG, "ESP_MSC_OTA_GET_IMG_DESC"); break;
    case ESP_MSC_OTA_VERIFY_CHIP_ID:
      esp_chip_id_t chip_id = *(esp_chip_id_t *)event_data;
      ESP_LOGI(TAG, "ESP_MSC_OTA_VERIFY_CHIP_ID, chip_id: %08x", chip_id);
      break;
    case ESP_MSC_OTA_UPDATE_BOOT_PARTITION:
      esp_partition_subtype_t subtype = *(esp_partition_subtype_t *)event_data;
      ESP_LOGI(TAG, "ESP_MSC_OTA_UPDATE_BOOT_PARTITION, subtype: %d", subtype);
      break;
    case ESP_MSC_OTA_FINISH:
      ESP_LOGI(TAG, "ESP_MSC_OTA_FINISH");
      syscfg_set(CFG_DATA, "MSC_OTA_MODE", "MSC");
      lv_label_set_text(ui_msc_ota_panel_bottom_Label, "USB를 제거 하세요");
      break;
    case ESP_MSC_OTA_ABORT:
      ESP_LOGI(TAG, "ESP_MSC_OTA_ABORT");
      syscfg_set(CFG_DATA, "MSC_OTA_MODE", "MSC");
      break;
  }
}

void usb_msc_ota_task(void) {
  LOGI(TAG, "Start usb msc ota task!!");
  esp_event_loop_create_default();
  ESP_ERROR_CHECK(esp_event_handler_register(ESP_MSC_OTA_EVENT, ESP_EVENT_ANY_ID, &msc_ota_event_handler, NULL));
  esp_msc_host_config_t msc_host_config = { .base_path = "/usb",
                                            .host_driver_config = DEFAULT_MSC_HOST_DRIVER_CONFIG(),
                                            .vfs_fat_mount_config = DEFAULT_ESP_VFS_FAT_MOUNT_CONFIG(),
                                            .host_config = DEFAULT_USB_HOST_CONFIG() };
  esp_msc_host_handle_t host_handle = NULL;
  esp_msc_host_install(&msc_host_config, &host_handle);

  esp_msc_ota_config_t config = {
    .ota_bin_path = OTA_FILE_NAME,
    .wait_msc_connect = portMAX_DELAY,
  };

#if CONFIG_SIMPLE_MSC_OTA
  esp_err_t ret = esp_msc_ota(&config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "esp_msc_ota failed, ret: %d", ret);
  }

#else

  esp_msc_ota_handle_t msc_ota_handle = NULL;

  esp_err_t ret = esp_msc_ota_begin(&config, &msc_ota_handle);
  ESP_GOTO_ON_ERROR(ret, fail, TAG, "esp_msc_ota_begin failed, err: %d", ret);

  do {
    ret = esp_msc_ota_perform(msc_ota_handle);
    if (ret != ESP_OK) {
      break;
      ESP_LOGE(TAG, "esp_msc_ota_perform: (%s)\n", esp_err_to_name(ret));
    }
    if (update_flag) {
      // sysevent_get(SYSEVENT_BASE, MSC_OTA_UPDATE, s_buf, sizeof(s_buf));
      sprintf(s_buf, "UADATE 진행률 : %d%%", ota_updata_percent);
      lv_label_set_text(ui_msc_ota_panel_top_Label, s_buf);
    }
  } while (!esp_msc_ota_is_complete_data_received(msc_ota_handle));

  if (esp_msc_ota_is_complete_data_received(msc_ota_handle)) {
    esp_msc_ota_end(msc_ota_handle);
    ESP_LOGI(TAG, "esp msc ota complete");
  } else {
    esp_msc_ota_abort(msc_ota_handle);
    ESP_LOGE(TAG, "esp msc ota failed");
  }
fail:
#endif
  esp_msc_host_uninstall(host_handle);
}

void create_usb_host_msc_ota_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;
  if (usb_msc_ota_handle) {
    LOGI(TAG, "usb host msc task is alreay created");
    return;
  }
  xTaskCreatePinnedToCore(usb_msc_ota_task, "usb_msc_ota", stack_size, NULL, task_priority, &usb_msc_ota_handle,
                          1);  // Core 1 yes, Core 0 No
}
