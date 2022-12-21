#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

static char s_uptime[128];

char* uptime(void) {
  struct timespec ts_now;

  clock_gettime(CLOCK_MONOTONIC, &ts_now);

  // Getting a milliseconds (ms)
  uint32_t now = ((uint64_t)ts_now.tv_sec * 1000) + ((uint64_t)ts_now.tv_nsec / 1000000L);
  uint32_t nearSec = (now / 1000);
  uint32_t hour = (nearSec / 3600);
  nearSec = nearSec - (hour * 3600);
  uint32_t minute = (nearSec / 60);
  nearSec = nearSec - (minute * 60);
  uint32_t second = nearSec;

  snprintf(s_uptime, sizeof(s_uptime), "%ld:%02ld:%02ld", hour, minute, second);

  return s_uptime;
}
