#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdio.h>
#include <stdbool.h>

#include "comm_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

bool send_command_data(message_type_t sender, message_type_t receiver, void *payload, size_t payload_len);

#ifdef __cplusplus
}
#endif

#endif  // _COMMAND_H_
