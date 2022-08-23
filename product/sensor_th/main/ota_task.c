#include "ota_fw.h"
#include "ota_task.h"
#include "config.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "log.h"

#define MAX_FW_DOWNLOAD_URL 256

static const char* TAG = "ota_task";

static TaskHandle_t ota_fw_handle;

static char* download_url = NULL;

void ota_fw_task(void* pParameters) {
  fw_ctx_t fwctx;

  LOGI(TAG, "Running OTA FW TASK !!!!");

  // set_wifi_led();
  // syscfg_set(CFG_DATA, FWUPGRADESTATUS, "1");

  if (download_url) {
    LOGI(TAG, "Firmware Download URL = %s", download_url);

    memset(&fwctx, 0, sizeof(fw_ctx_t));
    fwctx.download_url = download_url;

    ota_err_t err = ota_fw_download(&fwctx);
    if (err == OTA_OK || ota_fw_get_state(&fwctx) == OTA_STATE_DOWNLOAD) {
      LOGI(TAG, "FW image is activated !!!");
      ota_fw_active_new_image(&fwctx);
    } else if (err != OTA_OK) {
      LOGE(TAG, "Failed to OTA FW update !!!");
    }

    free(download_url);
  }

  // set_wifi_led();

  ota_fw_handle = NULL;
  vTaskDelete(NULL);
}

void start_ota_fw_task(char* fw_download_url) {
  uint16_t stack_size = SENS_OTAFW_TASK_STACK_SIZE;
  UBaseType_t task_priority = SENS_OTAFW_TASK_PRIORITY;

  if (ota_fw_handle) {
    LOGI(TAG, "OTA FW update task is alreay created");
    return;
  }
  if (!fw_download_url || strlen(fw_download_url) == 0) {
    LOGE(TAG, "Invalid fw_download_url");
    return;
  }

  download_url = calloc(1, MAX_FW_DOWNLOAD_URL);
  if (download_url) {
    snprintf(download_url, MAX_FW_DOWNLOAD_URL, "%s", fw_download_url);
    xTaskCreate((TaskFunction_t)ota_fw_task, "ota_fw_task", stack_size, NULL, task_priority, &ota_fw_handle);
  } else {
    LOGE(TAG, "Failed to calloc for download_url");
  }
}

void start_ota_fw_task_wait(char* fw_download_url) {}

