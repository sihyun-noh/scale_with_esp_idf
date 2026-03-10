#ifndef _UI_BRIDGE_H_
#define _UI_BRIDGE_H_

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "ui_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  QueueHandle_t ui_q;  // HW -> UI events go here
  QueueHandle_t hw_q;  // UI -> HW commands go here
} bridge_t;

/**
 * Initialize singleton instance.
 * Must be called once before any bridge_send_* usage.
 */
void bridge_init_singleton(QueueHandle_t ui_queue, QueueHandle_t hw_queue);

/**
 * Get singleton instance (read-only pointer).
 * Returns NULL if not initialized.
 */
const bridge_t *bridge_get(void);

/**
 * Convenience send functions.
 * Return true on success, false if not initialized or queue send fails.
 */
bool bridge_send_to_hw(ui_msg_id_t id, int32_t value, TickType_t to_ticks);
bool bridge_send_to_ui(ui_msg_id_t id, void *value, TickType_t to_ticks);

/**
 * Optional: check initialized
 */
bool bridge_is_ready(void);

#ifdef __cplusplus
}
#endif
#endif
