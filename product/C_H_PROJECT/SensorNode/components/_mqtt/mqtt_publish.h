
#ifndef MQTT_PUBLISH_H
#define MQTT_PUBLISH_H

#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Publish the device status to the JSON topic.
 *
 * This function creates a JSON object containing the device identifier
 * and a status string (e.g., "connected", "disconnected"), and publishes
 * it to the designated MQTT topic.
 *
 * @param client The MQTT client handle.
 * @param status The status string to include in the JSON payload.
 * @param topic string.
 */
void publish_device_status(esp_mqtt_client_handle_t client, const char *status, const char *topic);

/**
 * @brief Publish a command string to the command topic.
 *
 * This function wraps a command (e.g., "start", "stop") inside a JSON object
 * and publishes it to the command MQTT topic.
 *
 * @param client The MQTT client handle.
 * @param cmd The command string to include in the JSON payload.
 */
void publish_command(esp_mqtt_client_handle_t client, const char *cmd);

int publish_sensor_datatable(const char *topic, const char *data_table);

int publish_data(const char *topic, const char *json_data);

#ifdef __cplusplus
}
#endif

#endif  // MQTT_PUBLISH_H
