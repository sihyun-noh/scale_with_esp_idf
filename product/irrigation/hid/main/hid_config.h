#ifndef _HID_CONFIG_H_
#define _HID_CONFIG_H_

#include <stdbool.h>

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { SET_CONFIG, TIME_SYNC, VALVE_ON, VALVE_OFF, FLOW_STATUS, SET_SLEEP, RESPONSE, NONE } message_type_t;

typedef enum { SUCCESS, FAIL } response_type_t;

typedef struct config {
  int flow_rate;
  int zone_cnt;
  int zones[6];
  time_t start_time;
} config_t;

typedef int flow_status_t;
typedef bool set_valve_t;

typedef struct irrigation_message {
  message_type_t sender_type;
  message_type_t receiver_type;
  response_type_t resp;
  config_t config;
  flow_status_t flow;
  set_valve_t setting_value;
  int device_id;
  time_t current_time;
} irrigation_message_t;

void show_timestamp(time_t now);
bool read_hid_config(config_t *cfg);
bool save_hid_config(const char *flow, const char *start_time, const char *zones);

#ifdef __cplusplus
}
#endif

#endif /* _HID_CONFIG_H_ */
