
#include <stdio.h>
#include <string.h>

#include "esp32/icmp_echo_impl.h"

int ping_cmd(int argc, char **argv) {
  if (argv[1] == NULL) {
    return -1;
  }

  char *host = argv[1];

  // parse IP address
  struct sockaddr_in6 sock_addr6;
  ip_addr_t target_addr;
  memset(&target_addr, 0, sizeof(target_addr));
  if (inet_pton(AF_INET6, host, &sock_addr6.sin6_addr) == 1) {
    /* convert ip6 string to ip6 address */
    ipaddr_aton(host, &target_addr);
  } else {
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    /* convert ip4 string or hostname to ip4 or ip6 address */
    if (getaddrinfo(host, NULL, &hint, &res) != 0) {
      printf("ping: unknown host %s\n", host);
      return PING_ERR_UNKNOWN_HOST;
    }
    if (res->ai_family == AF_INET) {
      struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
      inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    } else {
      struct in6_addr addr6 = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr;
      inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
    }
    freeaddrinfo(res);
  }

  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  ping_config.target_addr = target_addr;

  /* set callback functions */
  esp_ping_callbacks_t cbs = { .cb_args = NULL,
                               .on_ping_success = cmd_ping_on_ping_success,
                               .on_ping_timeout = cmd_ping_on_ping_timeout,
                               .on_ping_end = cmd_ping_on_ping_end };
  esp_ping_handle_t ping;
  esp_ping_new_session(&ping_config, &cbs, &ping);
  esp_ping_start(ping);

  return PING_OK;
}
