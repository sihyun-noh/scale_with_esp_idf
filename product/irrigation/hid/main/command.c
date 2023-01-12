#include <string.h>

#include "command.h"
#include "comm_packet.h"
#include "espnow.h"
#include "time_api.h"

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

bool send_command_data(command_t cmd, void *payload, size_t payload_len) {
  irrigation_message_t message;

  memset(&message, 0x00, sizeof(message));

  switch (cmd) {
    case SET_CONFIG_COMMAND: {
      message.sender_type = SET_CONFIG;
      memcpy(&message.config, (config_value_t *)payload, payload_len);
      message.current_time = get_current_time();
    } break;
    case RESPONSE_COMMAND: {
      message.sender_type = RESPONSE;
      memcpy(&message.receive_type, (message_type_t *)payload, payload_len);
      message.resp = SUCCESS;
      message.current_time = get_current_time();
    } break;
    case REQ_TIME_SYNC_COMMAND: {
      message.sender_type = REQ_TIME_SYNC;
    } break;
    default: break;
  }

  return espnow_send_data(espnow_get_master_addr(), (uint8_t *)&message, sizeof(message));
}
