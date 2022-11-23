#ifndef _MQTTC_H_
#define _MQTTC_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The statuses emitted by the status callback.
 */
typedef enum mqtt_status_t { MQTT_STATUS_DISCONNECTED, MQTT_STATUS_CONNECTED } mqtt_status_t;

/**
 * The status callback.
 */
typedef void (*mqtt_status_callback_t)(mqtt_status_t);

/**
 * The message callback.
 */
typedef void (*mqtt_message_callback_t)(const char *topic, const uint8_t *payload, size_t len, int qos, bool retained);

/**
 * Initialize the MQTT management system.
 *
 * Note: Should only be called once on boot.
 *
 * @param scb - The status callback.
 * @param mcb - The message callback.
 * @param buffer_size - The read and write buffer size.
 * @param command_timeout - The command timeout.
 */
void lwmqtt_client_init(mqtt_status_callback_t scb, mqtt_message_callback_t mcb, size_t buffer_size,
                        int command_timeout);

/**
 * Configure TLS connection.
 *
 * The specified CA certificate is not copied and must be available during the whole duration of the TLS session.
 *
 * Note: This method must be called before `mqtt_start`.
 *
 * @param enable - Whether TLS should be used.
 * @param verify - Whether the connection should be verified.
 * @param ca_buf - The beginning of the CA certificate buffer.
 * @param ca_len - The length of the CA certificate buffer.
 * @return Whether TLS configuration was successful.
 */
bool lwmqtt_client_tls(bool enable, bool verify, const uint8_t *ca_buf, size_t ca_len);

/**
 * Configure Last Will and Testament.
 *
 * Note: Must be called before `mqtt_start`.
 *
 * @param topic - The LWT topic.
 * @param payload - The LWT payload.
 * @param qos - The LWT QoS level.
 * @param retained - The LWT retained flag.
 */
void lwmqtt_client_lwt(const char *topic, const char *payload, int qos, bool retained);

/**
 * Start the MQTT process.
 *
 * The background process will continuously attempt to connect to the specified broker. If a connection has been
 * established the status `MQTT_STATUS_CONNECTED` is emitted. Upon receiving this event the functions
 * `mqtt_subscribe`, `mqtt_unsubscribe` and `mqtt_publish` can be used to interact with the broker. If the
 * connection is lost the status `MQTT_STATUS_DISCONNECTED` es emitted and the process restarted. The process can be
 * stopped anytime by calling `mqtt_stop()`. However, if an established connection is stopped, the status
 * `MQTT_STATUS_DISCONNECTED` will not be emitted.
 *
 * @param host - The broker host.
 * @param port - The broker port.
 * @param client_id - The client id.
 * @param username - The client username.
 * @param password - The client password.
 * @return Whether the operation was successful.
 */
bool lwmqtt_client_start(const char *host, const char *port, const char *client_id, const char *username,
                         const char *password);

/**
 * Subscribe to specified topic.
 *
 * When false is returned the current operation failed and any subsequent interactions will also fail until a new
 * connection has been established and the status `MQTT_STATUS_CONNECTED` is emitted.
 *
 * @param topic - The topic.
 * @param qos - The qos level.
 * @return Whether the operation was successful.
 */
bool lwmqtt_client_subscribe(const char *topic, int qos);

/**
 * Unsubscribe from specified topic.
 *
 * When false is returned the current operation failed and any subsequent interactions will also fail until a new
 * connection has been established and the status `MQTT_STATUS_CONNECTED` is emitted.
 *
 * @param topic - The topic.
 * @return Whether the operation was successful.
 */
bool lwmqtt_client_unsubscribe(const char *topic);

/**
 * Publish bytes payload to specified topic.
 *
 * When false is returned the current operation failed and any subsequent interactions will also fail until a new
 * connection has been established and the status `MQTT_STATUS_CONNECTED` is emitted.
 *
 * @param topic - The topic.
 * @param payload - The payload.
 * @param len - The payload length.
 * @param qos - The qos level.
 * @param retained - The retained flag.
 * @return Whether the operation was successful.
 */
bool lwmqtt_client_publish(const char *topic, const uint8_t *payload, size_t len, int qos, bool retained);

/**
 * Stop the MQTT process.
 *
 * Will stop the running MQTT process.
 */
void lwmqtt_client_stop();

#ifdef __cplusplus
}
#endif

#endif  //_MQTTC_H_
