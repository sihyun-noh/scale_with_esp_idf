#ifndef TLS_LWMQTT_H_
#define TLS_LWMQTT_H_

#include <lwmqtt.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/platform.h>
#include <mbedtls/ssl.h>
#include <sdkconfig.h>

#include "log.h"

/**
 * The tls lwmqtt network object for the esp platform.
 */
typedef struct {
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_x509_crt cacert;
  mbedtls_net_context socket;
  uint8_t *ca_buf;
  size_t ca_len;
  bool verify;
} tls_lwmqtt_network_t;

/**
 * Initiate a connection to the specified remote host.
 */
lwmqtt_err_t tls_lwmqtt_network_connect(tls_lwmqtt_network_t *n, char *host, char *port);

/**
 * Wait until the socket is connected or a timeout has been reached.
 */
lwmqtt_err_t tls_lwmqtt_network_wait(tls_lwmqtt_network_t *n, bool *connected, uint32_t timeout);

/**
 * Terminate the connection.
 */
void tls_lwmqtt_network_disconnect(tls_lwmqtt_network_t *n);

/**
 * Will set available to the available amount of data in the underlying network buffer.
 */
lwmqtt_err_t tls_lwmqtt_network_peek(tls_lwmqtt_network_t *n, size_t *available, uint32_t timeout);

/**
 * Will wait for a socket until data is available or the timeout has been reached.
 */
lwmqtt_err_t tls_lwmqtt_network_select(tls_lwmqtt_network_t *n, bool *available, uint32_t timeout);

/**
 * The tls lwmqtt network read callback for the esp platform.
 */
lwmqtt_err_t tls_lwmqtt_network_read(void *ref, uint8_t *buf, size_t len, size_t *received, uint32_t timeout);

/**
 * The tls lwmqtt network write callback for the esp platform.
 */
lwmqtt_err_t tls_lwmqtt_network_write(void *ref, uint8_t *buf, size_t len, size_t *sent, uint32_t timeout);

#endif  // _TLS_LWMQTT_H_
