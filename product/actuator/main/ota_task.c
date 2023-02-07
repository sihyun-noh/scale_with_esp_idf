#include "ota_fw.h"
#include "ota_task.h"

// #include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "log.h"

#define MAX_FW_DOWNLOAD_URL 256

#define OTA_TASK_MSG_MASK 0xFFFF0000
#define OTA_TASK_MSG_ID 0xABAB0000
#define OTA_TASK_FWUP_OK 0x01
#define OTA_TASK_FWUP_FAIL 0x0F

/*
 * entry : 0x01: FW upgrade from Remote (MQTT), Need to send response of FW update result,
 *         0x02: FW upgrade from Local, No need to send response
 */
typedef struct {
  TaskHandle_t handle;
  unsigned int id;
  unsigned char entry;  // 0x01 : FW upgrade from Remote, 0x02 : FW upgrade from Local
} ota_task_params_t;

ota_task_params_t ota_task_params;
ota_task_params_t ota_task_params_wait;

static const char* TAG = "ota_task";

static TaskHandle_t ota_fw_handle;

static char* download_url = NULL;

void ota_fw_task(void* pParameters) {
  fw_ctx_t fwctx;
  ota_task_params_t* task_params = (ota_task_params_t*)pParameters;

  LOGI(TAG, "Running OTA FW TASK !!!!");

  // set_wifi_led();

  if (download_url) {
    LOGI(TAG, "Firmware Download URL = %s", download_url);

    memset(&fwctx, 0, sizeof(fw_ctx_t));
    fwctx.download_url = download_url;

    ota_err_t err = ota_fw_download(&fwctx);
    if (err == OTA_OK || ota_fw_get_state(&fwctx) == OTA_STATE_DOWNLOAD) {
      err = ota_fw_active_new_image(&fwctx);
      if (err == OTA_OK) {
        if (task_params->entry == 0x01) {
          xTaskNotify(task_params->handle, (task_params->id | OTA_TASK_FWUP_OK), eSetValueWithoutOverwrite);
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        LOGI(TAG, "New FW image is activated !!!");
      } else {
        if (task_params->entry == 0x01) {
          xTaskNotify(task_params->handle, (task_params->id | OTA_TASK_FWUP_FAIL), eSetValueWithoutOverwrite);
          vTaskDelay(pdMS_TO_TICKS(500));
        }
        LOGI(TAG, "New FW image is NOT activated !!!");
      }
    } else if (err != OTA_OK) {
      if (task_params->entry == 0x01) {
        xTaskNotify(task_params->handle, (task_params->id | OTA_TASK_FWUP_FAIL), eSetValueWithoutOverwrite);
        vTaskDelay(pdMS_TO_TICKS(500));
      }
      LOGE(TAG, "Failed to OTA FW update !!!");
    }

    free(download_url);
  }

  // set_wifi_led();

  // Wait for mqtt client to send FW update status.
  vTaskDelay(pdMS_TO_TICKS(10000));
  // Reset a device after updating FW image.
  ota_fw_reset_device(&fwctx);

  ota_fw_handle = NULL;
  vTaskDelete(NULL);
}

void start_ota_fw_task(char* fw_download_url) {
  if (ota_fw_handle) {
    LOGI(TAG, "OTA FW update task is alreay created");
    return;
  }
  if (!fw_download_url || strlen(fw_download_url) == 0) {
    LOGE(TAG, "Invalid fw_download_url");
    return;
  }

  ota_task_params.handle = NULL;
  ota_task_params.id = 0;
  ota_task_params.entry = 0x02;

  download_url = calloc(1, MAX_FW_DOWNLOAD_URL);
  if (download_url) {
    snprintf(download_url, MAX_FW_DOWNLOAD_URL, "%s", fw_download_url);
    xTaskCreate((TaskFunction_t)ota_fw_task, "ota_fw_task", 4096, &ota_task_params, (tskIDLE_PRIORITY + 5),
                &ota_fw_handle);
  } else {
    LOGE(TAG, "Failed to calloc for download_url");
  }
}

#define OTA_TIMEOUT_MS 120000
int start_ota_fw_task_wait(char* fw_download_url) {
  int ret = -1;

  if (ota_fw_handle) {
    LOGI(TAG, "OTA FW update task is alreay created");
    return ret;
  }
  if (!fw_download_url || strlen(fw_download_url) == 0) {
    LOGW(TAG, "fw_download_url is NOT valid!!!");
    return ret;
  }

  download_url = calloc(1, MAX_FW_DOWNLOAD_URL);
  if (download_url) {
    ota_task_params_wait.handle = xTaskGetCurrentTaskHandle();
    ota_task_params_wait.id = OTA_TASK_MSG_ID;
    ota_task_params_wait.entry = 0x01;

    xTaskNotifyStateClear(NULL);
    snprintf(download_url, MAX_FW_DOWNLOAD_URL, "%s", fw_download_url);
    if (xTaskCreate(ota_fw_task, (char const*)"ota_fw_task", 4096, &ota_task_params_wait, (tskIDLE_PRIORITY + 5),
                    &ota_fw_handle) != pdPASS) {
      LOGW(TAG, "[%s] Failed to create OTA FW Task", __FUNCTION__);
    } else {
      for (;;) {
        uint32_t id, value;
        uint32_t notification = 0;

        /* Wait for the result of mqtt publishing status */
        if (xTaskNotifyWait(ULONG_MAX, ULONG_MAX, &notification, pdMS_TO_TICKS(OTA_TIMEOUT_MS)) != pdTRUE) {
          LOGE(TAG, "[%s] OTA FW Task wait timeout", __FUNCTION__);
          break;
        }
        /* Check notification message */
        id = notification & OTA_TASK_MSG_MASK;
        if (id != OTA_TASK_MSG_ID) {
          continue;
        }
        /* Check notification value */
        value = notification & ~OTA_TASK_MSG_MASK;
        if (value == OTA_TASK_FWUP_OK) {
          LOGI(TAG, "[%s] OTA FW Task success", __FUNCTION__);
          ret = 0;
          break;
        } else {
          LOGI(TAG, "[%s] OTA FW Task Failed", __FUNCTION__);
          ret = -2;
          break;
        }
      }
    }
  } else {
    LOGE(TAG, "Failed to calloc for download_url");
  }

  return ret;
}
