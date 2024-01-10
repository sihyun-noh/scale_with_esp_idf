#ifndef _HID_CONFIG_H_
#define _HID_CONFIG_H_

#include <stdbool.h>

#include "comm_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { NONE_STAGE = 0, CURR_STAGE = 1, NEXT_STAGE = 2 } stage_t;

void show_timestamp(time_t now);
char *get_current_timestamp(void);
bool validation_of_start_irrigation_time(time_t start_time, stage_t *p_stage);
bool read_hid_config(config_value_t *cfg);
bool save_hid_config(const char *flow, const char *start_time_hour, const char *start_time_minute, const char *zones);

#ifdef __cplusplus
}
#endif

#endif /* _HID_CONFIG_H_ */
