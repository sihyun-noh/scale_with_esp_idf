#include "ota_fw.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "log.h"

static const char* TAG = "ota_task";

static TaskHandle_t ota_fw_handle;

void ota_fw_task(void* pParameters) {
  fw_ctx_t fwctx;

  LOGI(TAG, "Running OTA FW TASK !!!!");

  memset(&fwctx, 0, sizeof(fw_ctx_t));
  fwctx.download_url = "http://192.168.50.104:8000/SENS_SHT3x_0_1_0-f0362d-DVT.bin";

  ota_err_t err = ota_fw_download(&fwctx);
  if (err == OTA_OK || ota_fw_get_state(&fwctx) == OTA_STATE_DOWNLOAD) {
    LOGI(TAG, "FW image is activated !!!");
    ota_fw_active_new_image(&fwctx);
  } else if (err != OTA_OK) {
    LOGE(TAG, "Failed to OTA FW update !!!");
  }

  vTaskDelete(NULL);
}

void create_ota_fw_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (ota_fw_handle) {
    LOGI(TAG, "OTA FW update task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)ota_fw_task, "ota_fw_task", stack_size, NULL, task_priority, &ota_fw_handle);
}
