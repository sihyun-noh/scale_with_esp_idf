#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DAJILOG_USB_MEMORY_IDLE,
    DAJILOG_USB_MEMORY_WAITING,
    DAJILOG_USB_MEMORY_MOUNTED,
    DAJILOG_USB_MEMORY_VALIDATING,
    DAJILOG_USB_MEMORY_WRITING,
    DAJILOG_USB_MEMORY_READY_TO_REBOOT,
    DAJILOG_USB_MEMORY_ERROR,
} dajilog_usb_memory_update_state_t;

typedef struct {
    dajilog_usb_memory_update_state_t state;
    esp_err_t last_error;
    int progress_percent;
    char message[160];
} dajilog_usb_memory_update_status_t;

esp_err_t dajilog_usb_memory_update_start(void);
esp_err_t dajilog_usb_memory_update_apply(void);
esp_err_t dajilog_usb_memory_update_reboot(void);
void dajilog_usb_memory_update_get_status(dajilog_usb_memory_update_status_t *status);

#ifdef __cplusplus
}
#endif

