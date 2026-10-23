#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DAJILOG_PC_USB_UPDATE_IDLE = 0,
    DAJILOG_PC_USB_UPDATE_WAITING_FOR_USB,
    DAJILOG_PC_USB_UPDATE_USB_MOUNTED,
    DAJILOG_PC_USB_UPDATE_FILE_READY,
    DAJILOG_PC_USB_UPDATE_VALIDATING,
    DAJILOG_PC_USB_UPDATE_WRITING,
    DAJILOG_PC_USB_UPDATE_READY_TO_REBOOT,
    DAJILOG_PC_USB_UPDATE_ERROR,
} dajilog_pc_usb_update_state_t;

typedef struct {
    dajilog_pc_usb_update_state_t state;
    size_t file_size;
    size_t bytes_written;
    int progress_percent;
    esp_err_t last_error;
    char message[96];
} dajilog_pc_usb_update_status_t;

/** @brief Start the PC-facing USB MSC firmware update service. */
esp_err_t dajilog_pc_usb_update_start(void);
/** @brief Validate and install the firmware copied through PC USB MSC. */
esp_err_t dajilog_pc_usb_update_apply(void);
/** @brief Clear a recoverable error and return to update-ready state. */
esp_err_t dajilog_pc_usb_update_retry(void);
/** @brief Reboot only after a firmware image is ready to boot. */
esp_err_t dajilog_pc_usb_update_reboot(void);
/** @brief Copy current PC USB update progress and error state. */
void dajilog_pc_usb_update_get_status(dajilog_pc_usb_update_status_t *status);

#ifdef __cplusplus
}
#endif
