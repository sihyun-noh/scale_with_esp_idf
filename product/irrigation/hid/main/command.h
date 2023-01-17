#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SET_CONFIG_COMMAND,
  REQ_TIME_SYNC_COMMAND,
  RESPONSE_COMMAND,
  STOP_COMMAND,
} command_t;

bool send_command_data(command_t cmd, void *payload, size_t payload_len);

#ifdef __cplusplus
}
#endif

#endif  // _COMMAND_H_
