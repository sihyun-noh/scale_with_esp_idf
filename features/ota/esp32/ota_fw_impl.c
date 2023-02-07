#include "ota_fw_impl.h"
#include "log.h"
#include "config.h"
#include "syscfg.h"
#include "sys_config.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_image_format.h"

#define BUFFER_SIZE (32 * 1024)
#define FW_PROJECT_LEN 10

static const char *TAG = "OTA FW";

typedef enum {
  CHECKING_FIRMWARE = 0,
  VALID_FIRMWARE,
  INVALID_FIRMWARE,
  FLASHING_FIRMWARE,
} fw_status_t;

typedef struct {
  const fw_ctx_t *curr_fwctx;
  const esp_partition_t *update_partition;
  esp_ota_handle_t update_handle;
  ota_fw_state_t fw_state;
  uint32_t fw_image_len;
  uint32_t data_write_len;
  bool valid_fw_image;
} esp_ota_ctx_t;

static esp_ota_ctx_t ota_ctx;
static bool b_image_header_check;

static fw_status_t fw_status;

static char new_fw_version[SYSCFG_S_FWVERSION] = { 0 };

static void ota_ctx_clear(esp_ota_ctx_t *ctx) {
  if (ctx != NULL) {
    memset(ctx, 0, sizeof(esp_ota_ctx_t));
  }
}

static bool ota_ctx_validate(fw_ctx_t *fwctx) {
  return (fwctx != NULL && ota_ctx.curr_fwctx == fwctx && fwctx->ota_ctx == (uint8_t *)&ota_ctx);
}

static void ota_ctx_close(fw_ctx_t *fwctx) {
  if (fwctx != NULL) {
    fwctx->ota_ctx = 0;
  }
  ota_ctx.curr_fwctx = 0;
}

static int get_fw_image_status(void) {
  return fw_status;
}

static int check_fw_image(const esp_partition_t *running, uint8_t *fw_image, uint32_t read_image_len) {
  int fw_image_status = get_fw_image_status();
  if (b_image_header_check == false && fw_image_status == CHECKING_FIRMWARE) {
    esp_app_desc_t new_app_info;
    if (read_image_len > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
      // check current version and project name with the new firmware
      memcpy(&new_app_info, fw_image + sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t),
             sizeof(esp_app_desc_t));
      LOGI(TAG, "New firmware version: %s", new_app_info.version);
      LOGI(TAG, "New firmware project name: %s", new_app_info.project_name);

      // Save the new firmware version that will be used in MQTT fw update response.
      snprintf(new_fw_version, sizeof(new_fw_version), "%s", new_app_info.project_name);

      esp_app_desc_t running_app_info;
      if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        LOGI(TAG, "Running firmware version: %s", running_app_info.version);
        LOGI(TAG, "Running firmware project name: %s", running_app_info.project_name);
      }

      // If the FW name is different, firmware update will be ignored, it has been rollbacked to the previous fw
      // version
      if (strncasecmp(new_app_info.project_name, running_app_info.project_name, FW_PROJECT_LEN) != 0) {
        fw_image_status = INVALID_FIRMWARE;
        LOGI(TAG, "New firmware is invalid, This is not the firmware for this product");
        LOGW(TAG, "The firmware update will be ignored, it has been rollbacked to the previous firmware");
      } else {
        fw_image_status = VALID_FIRMWARE;

        const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
        if (last_invalid_app != NULL) {
          esp_app_desc_t invalid_app_info;
          if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
            LOGI(TAG, "Invalid firmware version: %s", invalid_app_info.version);
            LOGI(TAG, "Invalid firmware project name: %s", invalid_app_info.project_name);
          }
          if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(invalid_app_info.version)) == 0 &&
              memcmp(invalid_app_info.project_name, new_app_info.project_name, sizeof(invalid_app_info.project_name)) ==
                  0) {
            LOGI(TAG, "New firmware is invalid, last invalid firmware is the same as the new firmware");
            LOGW(TAG, "The firmware update will be ignored, it has been rollbacked to the previous firmware");
            fw_image_status = INVALID_FIRMWARE;
          }
        }
      }

      b_image_header_check = true;
    }
  } else if (b_image_header_check == true && fw_image_status == VALID_FIRMWARE) {
    fw_image_status = FLASHING_FIRMWARE;
  }

  return fw_image_status;
}

int ota_fw_abort_impl(fw_ctx_t *const fwctx) {
  int ret = OTA_ERR_ABORT;

  if (ota_ctx_validate(fwctx)) {
    ota_ctx_close(fwctx);
    ret = OTA_OK;
  } else if (fwctx && (fwctx->ota_ctx == NULL)) {
    ret = OTA_OK;
  }

  return ret;
}

int ota_fw_download_impl(fw_ctx_t *const fwctx) {
  int err = OTA_OK;

  ota_ctx.curr_fwctx = fwctx;
  fwctx->ota_ctx = (uint8_t *)&ota_ctx;

  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (configured != running) {
    LOGE(TAG, "Configured OTA boot partition and running OTA boot partition do not match");
    err = OTA_ERR_INVALID_STATE;
    goto err_match_partition;
  }

  LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);

  esp_http_client_config_t config = {
    .url = fwctx->download_url,
    .cert_pem = (char *)fwctx->certfile,
    .timeout_ms = 1000,
    .keep_alive_enable = true,
  };
  uint8_t *image_buffer = malloc(BUFFER_SIZE + 1);
  if (image_buffer == NULL) {
    LOGE(TAG, "Failed to alloc recieve buffer");
    err = OTA_ERR_BUFFER;
    goto err_alloc_buffer;
  }

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    LOGE(TAG, "Failed to initialise HTTP client");
    err = OTA_ERR_DOWNLOAD_IMAGE;
    goto err_http_client_init;
  }

  esp_err_t ret;
  if ((ret = esp_http_client_open(client, 0)) != ESP_OK) {
    LOGE(TAG, "Failed to open HTTP connection : %s", esp_err_to_name(err));
    err = OTA_ERR_DOWNLOAD_IMAGE;
    goto err_http_client_open;
  }

  int fw_image_len = esp_http_client_fetch_headers(client);
  LOGI(TAG, "fw_image_len = %d", fw_image_len);

  esp_ota_handle_t update_handle = 0;
  const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
  if (update_partition == NULL) {
    LOGE(TAG, "Failed to get next partition");
    err = OTA_ERR_INVALID_STATE;
    goto err_next_partition;
  }

  int total_data_read = 0, data_read;
  fw_status = CHECKING_FIRMWARE;

  ota_ctx.update_partition = update_partition;

  while (total_data_read < fw_image_len) {
    data_read = esp_http_client_read(client, (char *)image_buffer, BUFFER_SIZE);

    if (data_read == 0) {
      /*
       * As esp_http_client_read never returns negative error code, we rely on
       * `errno` to check for underlying transport connectivity closure if any
       */
      if (errno == ECONNRESET || errno == ENOTCONN) {
        LOGE(TAG, "Connection closed, errno = %d", errno);
        break;
      }
    } else if (data_read < 0) {
      LOGE(TAG, "Connection retry, errno = %d, data_read = %d", errno, data_read);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }
    image_buffer[data_read] = 0;

    fw_status = check_fw_image(running, image_buffer, data_read);
    if (fw_status == INVALID_FIRMWARE) {
      LOGE(TAG, "Firmware is invalid, it will be rollbacked to the previous firmware");
      break;
    } else if (fw_status == VALID_FIRMWARE) {
      LOGI(TAG, "Firmware is valid, it will be updated");
      esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
    }

    // Write fw image to ota flash area
    esp_ota_write(update_handle, (const void *)image_buffer, data_read);
    if (err != ESP_OK) {
      LOGE(TAG, "Failed to write FW image data");
      err = OTA_ERR_WRITE_FLASH;
      break;
    }

    total_data_read += data_read;
    LOGI(TAG, "total_data_read = %d, data_read = %d", total_data_read, data_read);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  if (fw_status == INVALID_FIRMWARE) {
    LOGI(TAG, "Invalid FW Image!!!");
    err = OTA_ERR_INVALID_IMAGE;
  } else {
    if (esp_http_client_is_complete_data_received(client)) {
      LOGI(TAG, "FW image donwload completed");
    } else {
      LOGI(TAG, "FW image donwload incompleted");
      if (err != OTA_ERR_WRITE_FLASH) {
        err = OTA_ERR_DOWNLOAD_IMAGE;
      }
    }
  }

  LOGI(TAG, "FW image reader status = %d, length = %d", esp_http_client_get_status_code(client),
       esp_http_client_get_content_length(client));

  esp_http_client_close(client);
  esp_http_client_cleanup(client);
  free(image_buffer);

  if (esp_ota_end(update_handle) != ESP_OK) {
    LOGE(TAG, "esp_ota_end failed!");
    err = OTA_ERR_WRITE_FLASH;
  }

  if (err == OTA_OK) {
    ota_ctx.valid_fw_image = true;
    ota_ctx.fw_state = OTA_FW_STATE_DOWNLOAD;
  } else {
    ota_ctx.valid_fw_image = false;
    ota_ctx.fw_state = OTA_FW_STATE_INVALID;
  }
  ota_ctx.fw_image_len = fw_image_len;
  ota_ctx.data_write_len = total_data_read;
  ota_ctx.update_handle = update_handle;

  return err;

err_next_partition:
  esp_http_client_close(client);
err_http_client_open:
  esp_http_client_cleanup(client);
err_http_client_init:
  free(image_buffer);
err_alloc_buffer:
err_match_partition:
  ota_ctx.fw_state = OTA_FW_STATE_ABORTED;
  return err;
}

int ota_fw_active_new_image_impl(fw_ctx_t *const fwctx) {
  int err = OTA_ERR_ACTIVATE_FAIL;

  if (ota_ctx_validate(fwctx)) {
    if (ota_ctx.valid_fw_image == false) {
      LOGE(TAG, "FW image updating is failed!");
      esp_partition_erase_range(ota_ctx.update_partition, 0, ota_ctx.update_partition->size);
    } else {
      esp_err_t err = esp_ota_set_boot_partition(ota_ctx.update_partition);
      if (err != ESP_OK) {
        LOGE(TAG, "esp_ota_set_boot_partition failded (%d)!", err);
        esp_partition_erase_range(ota_ctx.update_partition, 0, ota_ctx.update_partition->size);
      } else {
        err = OTA_OK;
        LOGI(TAG, "FW image is valid...");
        // Set new firmware version to fw_version syscfg variable after checking the FW validation.
        syscfg_set(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, new_fw_version);
        return err;
      }
    }
  } else {
    LOGI(TAG, "ota ctx(fwctx) is NOT valid...");
  }
  return err;
}

int ota_fw_reset_device_impl(fw_ctx_t *const fwctx) {
  ota_ctx_clear(&ota_ctx);
  vTaskDelay(500);
  esp_restart();
  return OTA_OK;
}

/**
 * @breif Get OTA FW update status
 */
ota_fw_state_t ota_fw_get_state_impl(fw_ctx_t *const fwctx) {
  esp_ota_img_states_t ota_state;
  ota_fw_state_t ota_fw_state = ota_ctx.fw_state;

  if ((ota_ctx.curr_fwctx != NULL) && (ota_ctx.fw_state == OTA_FW_STATE_DOWNLOAD)) {
    ota_state = ESP_OTA_IMG_NEW;
  } else if ((ota_ctx.curr_fwctx != NULL) && (ota_ctx.fw_state == OTA_FW_STATE_ABORTED)) {
    ota_state = ESP_OTA_IMG_ABORTED;
  } else {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_err_t ret = esp_ota_get_state_partition(running, &ota_state);
    if (ret != ESP_OK) {
      LOGI(TAG, "Failed to get_state_partition : ota_fw_state = %d", ota_fw_state);
      return ota_fw_state;
    }
  }

  switch (ota_state) {
    case ESP_OTA_IMG_PENDING_VERIFY: ota_fw_state = OTA_FW_STATE_PENDING; break;
    case ESP_OTA_IMG_VALID: ota_fw_state = OTA_FW_STATE_VALID; break;
    case ESP_OTA_IMG_NEW: ota_fw_state = OTA_FW_STATE_DOWNLOAD; break;
    case ESP_OTA_IMG_INVALID: ota_fw_state = OTA_FW_STATE_INVALID; break;
    case ESP_OTA_IMG_ABORTED: ota_fw_state = OTA_FW_STATE_ABORTED; break;
    default: ota_fw_state = OTA_FW_STATE_ABORTED; break;
  }

  LOGI(TAG, "ota_fw_state = %d", ota_fw_state);

  ota_ctx.fw_state = ota_fw_state;
  return ota_fw_state;
}

int ota_fw_set_state_impl(fw_ctx_t *const fwctx, ota_fw_state_t ota_fw_state) {
  if (ota_ctx_validate(fwctx)) {
    ota_ctx.fw_state = ota_fw_state;
  }
  return OTA_OK;
}
