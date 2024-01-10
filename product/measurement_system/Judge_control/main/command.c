#include <string.h>

#include "log.h"
#include "command.h"
#include "espnow.h"
#include "time_api.h"
#include "sysfile.h"
#include "filelog.h"
#include "comm_packet.h"

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

bool send_command_data(message_type_t sender, message_type_t receiver, void *payload, size_t payload_len) {
  irrigation_message_t message = { 0 };

  message.sender_type = sender;
  message.current_time = get_current_time();

  switch (sender) {
    case SET_CONFIG: {
      if (payload && payload_len) {
        memcpy(&message.payload, payload, payload_len);
        FDATA(BASE_PATH, "%s", "Send SET_CONFIG command");
      }
    } break;
    case RESPONSE: {
      message.receive_type = receiver;
    } break;
    case UPDATE_DEVICE_ADDR: {
      if (payload && payload_len) {
        memcpy(&message.payload, payload, payload_len);
      }
    } break;
    case REQ_TIME_SYNC: {
      FDATA(BASE_PATH, "%s", "Send Request Time sync command");
    } break;
    default: break;
  }

  LOG_BUFFER_HEXDUMP("COMMAND", (uint8_t *)&message, sizeof(irrigation_message_t), LOG_INFO);
  return espnow_send_data(espnow_get_master_addr(), (uint8_t *)&message, sizeof(irrigation_message_t));
}
