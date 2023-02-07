#ifndef _COMM_PACKET_H_
#define _COMM_PACKET_H_

#include "time.h"

typedef enum {
  SET_CONFIG = 0,
  TIME_SYNC,           // 1
  START_FLOW,          // 2
  BATTERY_LEVEL,       // 3
  SET_VALVE_ON,        // 4
  SET_VALVE_OFF,       // 5
  FLOW_STATUS,         // 6
  SET_SLEEP,           // 7
  RESPONSE,            // 8
  ZONE_COMPLETE,       // 9
  ALL_COMPLETE,        // 10, 0x0a
  FORCE_STOP,          // 11, 0x0b
  REQ_TIME_SYNC,       // 12, 0x0c
  DEVICE_ERROR,        // 13, 0x0d
  UPDATE_DEVICE_ADDR,  // 14, 0x0e
  NONE
} message_type_t;

// Keep enum value sequence for compatibility
typedef enum { HID_DEV = 0, CHILD_1, CHILD_2, CHILD_3, CHILD_4, CHILD_5, CHILD_6, MAIN_DEV, ALL_DEV } device_type_t;

typedef struct config_value {
  int flow_rate[6];
  int zone_cnt;
  int zones[6];
  time_t start_time;
} config_value_t;

typedef struct device_addr {
  device_type_t device_type;
  char mac_addr[13];
} device_addr_t;

typedef struct device_manage {
  device_addr_t update_dev_addr[7];  // main, child 1~6 ==> 7 devices
  int update_dev_cnt;
} device_manage_t;

typedef struct device_status {
  int flow_value;
  int deviceId;          // zone number or HID or master
  int battery_level[7];  // 0: HID, 1~6: child 1~6
  int child_status[6];   // 1~6: child 1~6, value 0 -> normal, other = error
} device_status_t;

typedef union payload {
  config_value_t config;
  device_status_t dev_stat;
  device_manage_t dev_manage;
  uint64_t remain_time_sleep;
} payload_t;

typedef enum {
  SUCCESS = 0,
  FAILURE = 1,
} response_type_t;

typedef struct irrigation_message {
  message_type_t sender_type;
  message_type_t receive_type;
  response_type_t resp;
  payload_t payload;
  time_t current_time;
} irrigation_message_t;

#endif  // _COMM_PACKET_H_
