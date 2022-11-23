#ifndef _LWMQTTC_H_
#define _LWMQTTC_H_

#include <lwmqtt.h>

/**
 * The lwmqtt timer object for the esp platform.
 */
typedef struct {
  uint32_t deadline;
} lwmqtt_timer_t;

/**
 * The lwmqtt timer set callback for the esp platform.
 */
void lwmqtt_timer_set(void *ref, uint32_t timeout);

/**
 * The lwmqtt timer get callback for the esp platform.
 */
int32_t lwmqtt_timer_get(void *ref);

/**
 * The lwmqtt network object for the esp platform.
 */
typedef struct {
  int socket;
} lwmqtt_network_t;

/**
 * Initiate a connection to the specified remote hose.
 */
lwmqtt_err_t lwmqtt_network_connect(lwmqtt_network_t *n, char *host, char *port);

/**
 * Wait until the socket is connected or a timeout has been reached.
 */
lwmqtt_err_t lwmqtt_network_wait(lwmqtt_network_t *n, bool *connected, uint32_t timeout);

/**
 * Terminate the connection.
 */
void lwmqtt_network_disconnect(lwmqtt_network_t *n);

/**
 * Will set available to the available amount of data in the underlying network buffer.
 */
lwmqtt_err_t lwmqtt_network_peek(lwmqtt_network_t *n, size_t *available);

/**
 * Will wait for a socket until data is available or the timeout has been reached.
 */
lwmqtt_err_t lwmqtt_network_select(lwmqtt_network_t *n, bool *available, uint32_t timeout);

/**
 * The lwmqtt network read callback for the esp platform.
 */
lwmqtt_err_t lwmqtt_network_read(void *ref, uint8_t *buf, size_t len, size_t *received, uint32_t timeout);

/**
 * The lwmqtt network write callback for the esp platform.
 */
lwmqtt_err_t lwmqtt_network_write(void *ref, uint8_t *buf, size_t len, size_t *sent, uint32_t timeout);

#endif  // _LWMQTT_H_
