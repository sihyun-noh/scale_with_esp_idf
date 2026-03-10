

#include "ui_bridge.h"
#include "esp_log.h"

static const char *TAG = "ui_bridge";
static bridge_t s_bridge;
static bool s_inited = false;

void bridge_init_singleton(QueueHandle_t ui_queue, QueueHandle_t hw_queue) {
  // Minimal safety: ignore re-init attempts if already initialized
  // (or you can assert here if you want strict behavior)
  if (s_inited) {
    return;
  }

  s_bridge.ui_q = ui_queue;
  s_bridge.hw_q = hw_queue;
  s_inited = true;
}

const bridge_t *bridge_get(void) {
  if (!s_inited)
    return NULL;
  return &s_bridge;
}

bool bridge_is_ready(void) {
  return s_inited && s_bridge.ui_q != NULL && s_bridge.hw_q != NULL;
}

static bool send_to_queue_int(QueueHandle_t q, ui_msg_id_t id, int32_t value, TickType_t to_ticks) {
  if (!q)
    return false;

  ui_msg_t msg = { .id = id, .value = value, .str = NULL };

  // xQueueSend is thread-safe
  return xQueueSend(q, &msg, to_ticks) == pdTRUE;
}

static bool send_to_queue_str(QueueHandle_t q, ui_msg_id_t id, char *value, TickType_t to_ticks) {
  if (!q)
    return false;

  ui_msg_t msg = { .id = id, .value = 0, .str = value };

  // xQueueSend is thread-safe
  return xQueueSend(q, &msg, to_ticks) == pdTRUE;
}

// send int32_t type
bool bridge_send_to_hw(ui_msg_id_t id, int32_t value, TickType_t to_ticks) {
  if (!bridge_is_ready())
    return false;
  return send_to_queue_int(s_bridge.hw_q, id, value, to_ticks);
}
// send str type
bool bridge_send_to_ui(ui_msg_id_t id, void *value, TickType_t to_ticks) {
  if (!bridge_is_ready())
    return false;
  return send_to_queue_str(s_bridge.ui_q, id, value, to_ticks);
}
