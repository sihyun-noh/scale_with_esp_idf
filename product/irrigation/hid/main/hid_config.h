#ifndef _HID_CONFIG_H_
#define _HID_CONFIG_H_

#include <stdbool.h>

#include "comm_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

void show_timestamp(time_t now);
bool read_hid_config(config_value_t *cfg);
bool save_hid_config(const char *flow, const char *start_time, const char *zones);

#ifdef __cplusplus
}
#endif

#endif /* _HID_CONFIG_H_ */
