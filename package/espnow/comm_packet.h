#ifndef _COMM_PACKET_H_
#define _COMM_PACKET_H_

#include "time.h"

typedef enum {
  SET_CONFIG = 0,
  TIME_SYNC,      // 1
  START_FLOW,     // 2
  BATTERY_LEVEL,  // 3
  SET_VALVE_ON,   // 4
  SET_VALVE_OFF,  // 5
  FLOW_STATUS,    // 6
  SET_SLEEP,      // 7
  RESPONSE,       // 8
  ZONE_COMPLETE,  // 9
  ALL_COMPLETE,   // 10, 0a
  FORCE_STOP,     // 11, 0b
  NONE
} message_type_t;

typedef struct config_value {
  int flow_rate[6];
  int zone_cnt;
  int zones[6];
  time_t start_time;
} config_value_t;

typedef enum {
  FAIL = 0,
  SUCCESS,
} response_type_t;

typedef struct irrigation_message {
  message_type_t sender_type;
  message_type_t receive_type;
  response_type_t resp;
  config_value_t config;
  int flow_value;
  int deviceId;  // zone number or HID or master
  uint64_t remain_time_sleep;
  int battery_level[7];  // 0: HID, 1~6: child 1~6
  time_t current_time;
} irrigation_message_t;

#endif  // _COMM_PACKET_H_
