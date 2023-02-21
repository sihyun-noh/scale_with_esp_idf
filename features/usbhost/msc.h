#ifndef _MSC_H_
#define _MSC_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  HOST_NO_CLIENT = 0x1,
  HOST_ALL_FREE = 0x2,
  DEVICE_CONNECTED = 0x4,
  DEVICE_DISCONNECTED = 0x8,
  DEVICE_ADDRESS_MASK = 0xFF0,
} app_event_t;

/**
 * @brief Waiting for device connection
 *
 */

uint8_t wait_for_msc_device(void);

/**
 * @brief event handler.
 *
 */

bool wait_for_event(app_event_t event_stat, uint32_t time_ms);

/**
 * @brief Initialize usb host msc
 *
 */

int usb_msc_host_init(void);

/**
 * @brief Uninitialize usb host msc
 *
 */
int usb_msc_host_uninit(void);

/**
 * @brief Initialize usb device.
 *
 */
int usb_device_init(uint8_t device_address);

/**
 * @brief Uninitialize usb device.
 *
 */
int usb_device_uninit(void);

// int file_operations(void);

#ifdef __cplusplus
}
#endif

#endif /* _MSC_H_ */
