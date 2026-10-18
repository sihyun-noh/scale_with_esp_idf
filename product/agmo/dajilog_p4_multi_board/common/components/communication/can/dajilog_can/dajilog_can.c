#include "dajilog_can.h"

#include <string.h>
#include <stdatomic.h>

#include "driver/twai.h"
#include "esp_log.h"

static const char *TAG = "DajilogCAN";

static bool s_installed;
static atomic_uint_least32_t s_total_rx;
static atomic_uint_least32_t s_last_rx_id;
static atomic_uchar s_last_rx_dlc;
static atomic_bool s_last_rx_extended;

dajilog_can_filter_t dajilog_can_filter_accept_all(void) {
  return (dajilog_can_filter_t){
    .acceptance_code = 0,
    .acceptance_mask = UINT32_MAX,
    .single_filter = true,
  };
}

esp_err_t dajilog_can_init(const dajilog_can_config_t *config) {
  if (s_installed) {
    return ESP_ERR_INVALID_STATE;
  }
  if (config == NULL || config->tx_gpio < 0 || config->rx_gpio < 0) {
    return ESP_ERR_INVALID_ARG;
  }

  twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(config->tx_gpio, config->rx_gpio, TWAI_MODE_NORMAL);
  general.tx_queue_len = config->tx_queue_length;
  general.rx_queue_len = config->rx_queue_length;

  twai_timing_config_t timing;
  if (config->bitrate == 500000U) {
    const twai_timing_config_t value = TWAI_TIMING_CONFIG_500KBITS();
    timing = value;
  } else if (config->bitrate == 250000U) {
    const twai_timing_config_t value = TWAI_TIMING_CONFIG_250KBITS();
    timing = value;
  } else if (config->bitrate == 1000000U) {
    const twai_timing_config_t value = TWAI_TIMING_CONFIG_1MBITS();
    timing = value;
  } else {
    return ESP_ERR_NOT_SUPPORTED;
  }

  const twai_filter_config_t filter = {
    .acceptance_code = config->filter.acceptance_code,
    .acceptance_mask = config->filter.acceptance_mask,
    .single_filter = config->filter.single_filter,
  };
  const esp_err_t result = twai_driver_install(&general, &timing, &filter);
  if (result == ESP_OK) {
    s_installed = true;
    atomic_store(&s_total_rx, 0);
    atomic_store(&s_last_rx_id, 0);
    atomic_store(&s_last_rx_dlc, 0);
    atomic_store(&s_last_rx_extended, false);
    ESP_LOGI(TAG, "TWAI installed: TX GPIO%d, RX GPIO%d, bitrate %lu", config->tx_gpio, config->rx_gpio,
             (unsigned long)config->bitrate);
  } else {
    ESP_LOGE(TAG, "TWAI install failed: %s", esp_err_to_name(result));
  }
  return result;
}

esp_err_t dajilog_can_start(void) {
  if (!s_installed) {
    return ESP_ERR_INVALID_STATE;
  }
  const esp_err_t result = twai_start();
  if (result == ESP_OK) {
    ESP_LOGI(TAG, "TWAI started");
  } else {
    ESP_LOGE(TAG, "TWAI start failed: %s", esp_err_to_name(result));
  }
  return result;
}

esp_err_t dajilog_can_stop(void) {
  return s_installed ? twai_stop() : ESP_ERR_INVALID_STATE;
}

esp_err_t dajilog_can_deinit(void) {
  if (!s_installed) {
    return ESP_ERR_INVALID_STATE;
  }
  const esp_err_t result = twai_driver_uninstall();
  if (result == ESP_OK) {
    s_installed = false;
  }
  return result;
}

esp_err_t dajilog_can_write(const dajilog_can_frame_t *frame, TickType_t timeout) {
  if (!s_installed || frame == NULL || frame->dlc > 8U) {
    return ESP_ERR_INVALID_ARG;
  }
  twai_message_t message = { 0 };
  message.identifier = frame->identifier;
  message.data_length_code = frame->dlc;
  message.extd = frame->extended;
  message.rtr = frame->remote;
  memcpy(message.data, frame->data, frame->dlc);
  return twai_transmit(&message, timeout);
}

esp_err_t dajilog_can_read(dajilog_can_frame_t *frame, TickType_t timeout) {
  if (!s_installed || frame == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  twai_message_t message = { 0 };
  const esp_err_t result = twai_receive(&message, timeout);
  if (result != ESP_OK) {
    return result;
  }
  memset(frame, 0, sizeof(*frame));
  frame->identifier = message.identifier;
  frame->dlc = message.data_length_code > 8U ? 8U : message.data_length_code;
  frame->extended = message.extd;
  frame->remote = message.rtr;
  memcpy(frame->data, message.data, frame->dlc);
  atomic_fetch_add(&s_total_rx, 1);
  atomic_store(&s_last_rx_id, frame->identifier);
  atomic_store(&s_last_rx_dlc, frame->dlc);
  atomic_store(&s_last_rx_extended, frame->extended);
  return ESP_OK;
}

esp_err_t dajilog_can_get_status(dajilog_can_status_t *status) {
  if (!s_installed || status == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  twai_status_info_t info;
  const esp_err_t result = twai_get_status_info(&info);
  if (result != ESP_OK) {
    return result;
  }
  status->state = (dajilog_can_state_t)info.state;
  status->pending_tx = info.msgs_to_tx;
  status->pending_rx = info.msgs_to_rx;
  status->tx_failed = info.tx_failed_count;
  status->rx_missed = info.rx_missed_count;
  status->bus_errors = info.bus_error_count;
  status->total_rx = atomic_load(&s_total_rx);
  status->last_rx_id = atomic_load(&s_last_rx_id);
  status->last_rx_dlc = atomic_load(&s_last_rx_dlc);
  status->last_rx_extended = atomic_load(&s_last_rx_extended);
  return ESP_OK;
}

esp_err_t dajilog_can_recover(void) {
  return s_installed ? twai_initiate_recovery() : ESP_ERR_INVALID_STATE;
}
