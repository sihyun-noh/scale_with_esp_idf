#ifndef _OTA_FW_IMPL_H_
#define _OTA_FW_IMPL_H_

#include "ota_fw.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @breif OTA FW image state
 */
typedef enum ota_fw_state {
  OTA_FW_STATE_UNKNWON = 0,
  OTA_FW_STATE_DOWNLOAD,
  OTA_FW_STATE_PENDING,
  OTA_FW_STATE_VALID,
  OTA_FW_STATE_INVALID,
  OTA_FW_STATE_ABORTED,
} ota_fw_state_t;

int ota_fw_abort_impl(fw_ctx_t* const fwctx);

int ota_fw_download_impl(fw_ctx_t* const fwctx);

int ota_fw_active_new_image_impl(fw_ctx_t* const fwctx);

int ota_fw_reset_device_impl(fw_ctx_t* const fwctx);

ota_fw_state_t ota_fw_get_state_impl(fw_ctx_t* const fwctx);

int ota_fw_set_state_impl(fw_ctx_t* const fwctx, ota_fw_state_t fw_state);

#ifdef __cplusplus
}
#endif

#endif /* _OTA_FW_IMPL_H_ */
