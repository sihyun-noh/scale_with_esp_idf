#ifndef _COMM_PACKET_H_
#define _COMM_PACKET_H_

#include "time.h"

typedef enum {
  SET_CONFIG = 0,
  TIME_SYNC,
  START_FLOW,
  BATTERY_LEVEL,
  SET_VALVE_ON,
  SET_VALVE_OFF,
  FLOW_STATUS,
  SET_SLEEP,
  RESPONSE,
  ZONE_COMPLETE,
  ALL_COMPLETE,
  NONE
} message_type_t;

typedef struct config_value {
  int flow_rate;
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
