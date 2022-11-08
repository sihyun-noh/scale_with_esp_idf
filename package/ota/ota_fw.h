#ifndef _OTA_FW_H_
#define _OTA_FW_H_

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ota_err {
  OTA_OK = 0,
  OTA_ERR_ABORT = -1,
  OTA_ERR_BUFFER = -2,
  OTA_ERR_INVALID_STATE = -3,
  OTA_ERR_INVALID_IMAGE = -4,
  OTA_ERR_INVALID_IMAGE_SIZE = -5,
  OTA_ERR_WRITE_FLASH = -6,
  OTA_ERR_DOWNLOAD_IMAGE = -7,
  OTA_ERR_ACTIVATE_FAIL = -8,
} ota_err_t;

typedef enum ota_state {
  OTA_STATE_IDLE = 0,
  OTA_STATE_DOWNLOAD = 1,
  OTA_STATE_PENDING = 2,
  OTA_STATE_ACCEPTED = 3,
  OTA_STATE_REJECTED = 4,
  OTA_STATE_ABORTED = 5,
} ota_state_t;

/**
 * @brief The context of the ota fw update.
 */
typedef struct fw_ctx {
  char *download_url;
  uint8_t *ota_ctx;
  uint8_t *certfile;
  uint32_t image_size;
  uint32_t image_type;
} fw_ctx_t;

/**
 * @brief Abort the OTA FW process if not needed
 *
 */
ota_err_t ota_fw_abort(fw_ctx_t *fwctx);

/**
 * @brief Download the OTA FW image from the FW update server
 *
 */
ota_err_t ota_fw_download(fw_ctx_t *fwctx);

/**
 * @brief Set active new FW image after downloading the OTA FW image
 *
 */
ota_err_t ota_fw_active_new_image(fw_ctx_t *fwctx);

/**
 * @brief Reset the ESP32 to boot the new FW image
 *
 */
ota_err_t ota_fw_reset_device(fw_ctx_t *fwctx);

/**
 * @brief Get the current OTA FW image state
 *
 */
ota_state_t ota_fw_get_state(fw_ctx_t *fwctx);

/**
 * @brief Set the current OTA FW image state
 *
 */
ota_err_t ota_fw_set_state(fw_ctx_t *fwctx, ota_state_t ota_state);

#ifdef __cplusplus
}
#endif

#endif /* _OTA_FW_H_ */
