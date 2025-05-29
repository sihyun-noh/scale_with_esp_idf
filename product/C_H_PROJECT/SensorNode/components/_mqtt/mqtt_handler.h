
#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle JSON-formatted status data received from the MQTT topic.
 *
 * This function parses the JSON payload and logs or processes fields
 * such as "device" and "status".
 *
 * @param payload The JSON-formatted string received via MQTT.
 */
void handle_json_payload(const char *payload);

/**
 * @brief Handle command data received from the MQTT topic.
 *
 * This function parses a JSON payload that includes a command string (e.g., "start"),
 * and performs related logic or logs the command.
 *
 * @param payload The JSON-formatted string containing the command.
 */
void handle_command_payload(const char *payload);

#ifdef __cplusplus
}
#endif

#endif  // MQTT_HANDLER_H
