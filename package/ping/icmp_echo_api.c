
#include <stdio.h>
#include <string.h>

#include "icmp_echo_api.h"

int do_ping(char *host, uint8_t ping_count) {
  // return do_ping_impl(host, ping_count);
  return do_ping_lwip_impl(host, ping_count);
}
