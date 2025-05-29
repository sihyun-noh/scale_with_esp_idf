
#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

// Version
#define MQTT_VERSION "v1"

// General prefixes
#define TOPIC_PREFIX MQTT_VERSION "/device"  // ex) v1/device

// Subtopics
#define TOPIC_STATE   "state"
#define TOPIC_CONFIG  "config"
#define TOPIC_SENSOR  "sensor"
#define TOPIC_CMD     "cmd"
#define TOPIC_JSON    "json"
#define TOPIC_OTA     "ota"
#define TOPIC_SETTING "setting"
#define TOPIC_UPLOAD  "upload"
//
#define MAX_TOPIC_LEN     128
#define MAX_PAYLOAD_LEN   512
#define MAX_STRATEGIES    10
#define MAX_TASK_NAME_LEN 20

/**
 * @brief Macro to build a device-specific MQTT topic string.
 *
 * This macro simplifies the call to `build_device_topic_ext()` by automatically
 * passing the size of the provided buffer. It generates a topic string in the
 * format: `v1/device/{device_id}/{topic_type}`.
 *
 * @param buf  Output buffer to store the generated topic string.
 * @param type Topic type enum value (e.g., TOPIC_TYPE_STATE, TOPIC_TYPE_CONFIG).
 *
 * @note The `buf` parameter must be a character array, not a pointer, so that
 *       `sizeof(buf)` correctly resolves to the buffer size.
 */
#define BUILD_DEVICE_TOPIC(buf, type) build_device_topic_ext((type), (buf), sizeof(buf))

typedef enum {
  TOPIC_TYPE_STATE,
  TOPIC_TYPE_CONFIG,
  TOPIC_TYPE_SENSOR,
  TOPIC_TYPE_CMD,
  TOPIC_TYPE_JSON,
  TOPIC_TYPE_OTA,
  TOPIC_TYPE_SETTING,
  TOPIC_TYPE_UPLOAD
} topic_type_t;

typedef struct {
  esp_mqtt_client_handle_t client;
} mqtt_client_ctx_t;

typedef struct {
  char topic[MAX_TOPIC_LEN];
  char payload[MAX_PAYLOAD_LEN];
} mqtt_message_t;

/**
 * @brief Function pointer type for a strategy handler.
 *
 * A strategy function processes an MQTT message based on its topic and payload.
 *
 * @param msg Pointer to the received MQTT message.
 */
typedef void (*strategy_fn_t)(const mqtt_message_t *);

typedef struct {
  char topic_prefix[MAX_TOPIC_LEN];
  strategy_fn_t strategy;
  char task_name[MAX_TASK_NAME_LEN];
} strategy_entry_t;

typedef struct {
  strategy_entry_t table[MAX_STRATEGIES];
  int count;
} strategy_manager_t;

/**
 * @brief Returns the singleton instance of the strategy manager.
 *
 * @return Pointer to the global strategy_manager_t instance.
 */
strategy_manager_t *strategy_manager_get_instance(void);

/**
 * @brief Retrieves the name of the task responsible for handling a given MQTT topic.
 *
 * This function searches the registered strategy table for a topic prefix
 * that matches the provided topic, and returns the corresponding task name
 * associated with the matching strategy.
 *
 * @param topic The full MQTT topic string received (e.g., "/relay/on").
 * @return The name of the task associated with the topic if found; otherwise, NULL.
 */
const char *strategy_manager_get_task_name_for_topic(const char *topic);

/**
 * @brief Type definition for a topic-based queue routing function.
 *
 * A function of this type takes an MQTT topic string and returns
 * the corresponding message queue handle for the appropriate task.
 */
typedef QueueHandle_t (*topic_router_fn_t)(const char *topic);

/**
 * @brief Registers a topic-to-queue mapper function for RouterTask.
 *
 * This function allows external modules (e.g., TaskManager) to provide
 * a callback that RouterTask can use to route incoming MQTT messages
 * to the correct task queue based on the topic.
 *
 * @param fn Function pointer that implements topic-to-queue mapping logic.
 */
void router_register_queue_mapper(topic_router_fn_t fn);

/**
 * @brief Registers a new strategy function for a specific topic prefix and task.
 *
 * Adds a new entry to the strategy manager's table, associating a topic prefix
 * with a strategy function and the name of the task that should handle it.
 *
 * @param topic_prefix The MQTT topic prefix to match (e.g., "/relay/on").
 * @param task_name The name of the task responsible for handling the strategy.
 * @param fn The strategy function to execute when a matching topic is received.
 * @return true if the strategy was successfully registered; false if the table is full.
 */

bool strategy_manager_register(const char *topic_prefix, const char *task_name, strategy_fn_t fn);
/**
 * @brief Finds a registered strategy function that matches the given MQTT topic.
 *
 * Searches the strategy table for an entry whose topic prefix matches the provided topic,
 * and returns the corresponding strategy function pointer.
 *
 * @param topic The full MQTT topic string to match against registered strategy prefixes.
 * @return A pointer to the matching strategy function, or NULL if no match is found.
 */
strategy_fn_t strategy_manager_find_strategy(const char *topic);

/**
 * @brief Initialize and start the MQTT client.
 *
 * This function sets up the MQTT client with the configured broker URI,
 * registers the event handler for MQTT events, and starts the client.
 * It should be called once during application startup to establish
 * MQTT communication.
 */
void mqtt_app_start(void);

/**
 * @brief Router task for dispatching MQTT messages.
 *
 * This FreeRTOS task continuously waits for incoming MQTT messages
 * and routes them to the appropriate handler based on the strategy
 * registered for the corresponding topic.
 *
 * @param param Pointer to a queue handle or context (if needed).
 */
void router_task(void *param);

/**
 * @brief Build MQTT topic string in the format: v1/device/{device_id}/{topic_type}.
 *
 * @param type     Topic type (e.g., state, config, sensor, etc.)
 * @param buf      Output buffer to store the generated topic string.
 * @param len      Length of the output buffer.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG or other error on failure.
 */
esp_err_t build_device_topic_ext(topic_type_t type, char *buf, size_t len);

/**
 * @brief Convert topic type enum to string.
 *
 * Maps a topic_type_t enum to its corresponding topic name string (e.g., "state").
 *
 * @param type The topic type enum value.
 * @return     Corresponding string (e.g., "state", "config"), or "unknown" if invalid.
 */
const char *topic_type_to_str(topic_type_t type);

#ifdef __cplusplus
}
#endif

#endif  // MQTT_CONFIG_H
