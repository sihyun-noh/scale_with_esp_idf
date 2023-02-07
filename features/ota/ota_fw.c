/**
 * @file otw_fw.c
 *
 * @brief The ota firmware provides the ability to update new fw image while the normal firmware is running
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "ota_fw.h"
#include "esp32/ota_fw_impl.h"
#include "esp_ota_ops.h"

ota_err_t ota_fw_abort(fw_ctx_t *fwctx) {
  return ota_fw_abort_impl(fwctx);
}

ota_err_t ota_fw_download(fw_ctx_t *fwctx) {
  return ota_fw_download_impl(fwctx);
}

ota_err_t ota_fw_active_new_image(fw_ctx_t *fwctx) {
  return ota_fw_active_new_image_impl(fwctx);
}

ota_err_t ota_fw_reset_device(fw_ctx_t *fwctx) {
  return ota_fw_reset_device_impl(fwctx);
}

ota_state_t ota_fw_get_state(fw_ctx_t *fwctx) {
  ota_state_t ota_state;
  ota_fw_state_t fw_state = ota_fw_get_state_impl(fwctx);
  switch (fw_state) {
    case OTA_FW_STATE_VALID: ota_state = OTA_STATE_ACCEPTED; break;
    case OTA_FW_STATE_INVALID: ota_state = OTA_STATE_REJECTED; break;
    case OTA_FW_STATE_PENDING: ota_state = OTA_STATE_PENDING; break;
    case OTA_FW_STATE_DOWNLOAD: ota_state = OTA_STATE_DOWNLOAD; break;
    default: ota_state = OTA_STATE_ABORTED; break;
  }
  return ota_state;
}

ota_err_t ota_fw_set_state(fw_ctx_t *fwctx, ota_state_t ota_state) {
  ota_fw_state_t fw_state;

  switch (ota_state) {
    case OTA_STATE_PENDING:
    case OTA_STATE_ACCEPTED:
      fw_state = OTA_FW_STATE_VALID;
      esp_ota_mark_app_valid_cancel_rollback();
      break;
    case OTA_STATE_REJECTED:
      fw_state = OTA_FW_STATE_INVALID;
      esp_ota_mark_app_invalid_rollback_and_reboot();
      break;
    default: fw_state = OTA_STATE_ABORTED; break;
  }
  return ota_fw_set_state_impl(fwctx, fw_state);
}
