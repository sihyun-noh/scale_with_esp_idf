
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "esp_err.h"
#include "sensor_cfg_config.h"

#define TOPIC_MAX_LEN    128
#define MAC_ADDR_MAX_LEN 32

typedef struct {
  int port;                    ///< port number (1~6)
  char mac[MAC_ADDR_MAX_LEN];  ///< MAC address string
} topic_info_t;

/**
 * @brief Extract port number and MAC address from MQTT topic like:
 *        "v1/device/{mac}/port/{n}/config"
 *
 * @param topic Null-terminated MQTT topic string
 * @param[out] info Struct to fill with extracted MAC and port
 * @return ESP_OK on success, ESP_FAIL on format error
 */

esp_err_t parse_topic_info(const char *topic, topic_info_t *info) {
  if (!topic || !info)
    return ESP_FAIL;

  // 기대 형식: v1/device/{mac}/port/{n}/config
  const char *p = strstr(topic, "v1/device/");
  if (!p)
    return ESP_FAIL;

  p += strlen("v1/device/");  // MAC 시작 위치
  const char *mac_end = strchr(p, '/');
  if (!mac_end || mac_end - p >= MAC_ADDR_MAX_LEN)
    return ESP_FAIL;

  strncpy(info->mac, p, mac_end - p);
  info->mac[mac_end - p] = '\0';

#if 0
  // 이제 /port/{n}/ 찾기
  const char *port_ptr = strstr(mac_end, "/port/");
  if (!port_ptr)
    return ESP_FAIL;

  port_ptr += strlen("/port/");
  int port = atoi(port_ptr);
  if (port < 1 || port > SENSOR_PORT_COUNT)
    return ESP_FAIL;

  info->port = port;
#endif
  return ESP_OK;
}

/**
 * @brief Construct MQTT topic for config or state messages
 *        Format: v1/device/{device_id}/port/{n}/config or state
 *
 * @param port Port number (1~6)
 * @param is_state true for "state", false for "config"
 * @param[out] topic_buf Output buffer
 * @param buf_len Length of topic_buf
 * @return ESP_OK or error
 */
esp_err_t build_device_topic(int port, bool is_state, char *topic_buf, size_t buf_len) {
  if (!topic_buf || port < 1 || port > SENSOR_PORT_COUNT)
    return ESP_ERR_INVALID_ARG;

  char *device_id = NULL;
  esp_err_t err = get_device_id(&device_id);
  if (err != ESP_OK)
    return err;

  snprintf(topic_buf, buf_len, "v1/device/%s/port/%d/%s", device_id, port, is_state ? "state" : "config");

  free(device_id);
  return ESP_OK;
}

/**
 * @brief Parse MQTT topic and extract port number and device_id.
 *        Supports format: v1/device/{device_id}/port/{n}/config or state
 *
 * @param topic MQTT topic string
 * @param[out] out_port Parsed port number
 * @param[out] out_mac MAC string (optional)
 * @param max_mac_len Length of out_mac buffer
 * @return ESP_OK or error
 */
esp_err_t parse_device_topic(const char *topic, int *out_port, char *out_mac, size_t max_mac_len) {
  if (!topic || !out_port)
    return ESP_ERR_INVALID_ARG;

  const char *p = strstr(topic, "v1/device/");
  if (!p)
    return ESP_FAIL;
  p += strlen("v1/device/");

  const char *mac_end = strchr(p, '/');
  if (!mac_end || mac_end - p >= max_mac_len)
    return ESP_FAIL;

  if (out_mac) {
    strncpy(out_mac, p, mac_end - p);
    out_mac[mac_end - p] = '\0';
  }

  const char *port_ptr = strstr(mac_end, "/port/");
  if (!port_ptr)
    return ESP_FAIL;
  port_ptr += strlen("/port/");

  int port = atoi(port_ptr);
  if (port < 1 || port > SENSOR_PORT_COUNT)
    return ESP_FAIL;

  *out_port = port;
  return ESP_OK;
}
