#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include "syscfg.h"

#include <cJSON.h>

// syscfg for configuration data

#define SYSCFG_N_SSID "ssid"
#define SYSCFG_S_SSID 32
#define SYSCFG_I_SSID CFG_DATA

#define SYSCFG_N_PASSWORD "password"
#define SYSCFG_S_PASSWORD 60
#define SYSCFG_I_PASSWORD CFG_DATA

#define SYSCFG_N_FARMIP "farmip"
#define SYSCFG_S_FARMIP 80
#define SYSCFG_I_FARMIP CFG_DATA

#define SYSCFG_N_GATEWAYIP "gateway"
#define SYSCFG_S_GATEWAYIP 16
#define SYSCFG_I_GATEWAYIP CFG_DATA

#define SYSCFG_N_FWVERSION "fw_version"
#define SYSCFG_S_FWVERSION 80
#define SYSCFG_I_FWVERSION CFG_DATA

#define SYSCFG_N_DEVICEID "deviceid"
#define SYSCFG_S_DEVICEID 48
#define SYSCFG_I_DEVICEID CFG_DATA

#define SYSCFG_N_TIMEZONE "timezone"
#define SYSCFG_S_TIMEZONE 20
#define SYSCFG_I_TIMEZONE CFG_DATA

#define SYSCFG_N_RECONNECT "reconnect"
#define SYSCFG_S_RECONNECT 20
#define SYSCFG_I_RECONNECT CFG_DATA

#define SYSCFG_N_SEND_INTERVAL "send_interval"
#define SYSCFG_S_SEND_INTERVAL 10
#define SYSCFG_I_SEND_INTERVAL CFG_DATA

// syscfg for manufacturing data

#define SYSCFG_N_HWVERSION "hw_version"
#define SYSCFG_S_HWVERSION 10
#define SYSCFG_I_HWVERSION MFG_DATA

#define SYSCFG_N_POWERMODE "power_mode"
#define SYSCFG_S_POWERMODE 10
#define SYSCFG_I_POWERMODE MFG_DATA

#define SYSCFG_N_MODELNAME "model_name"
#define SYSCFG_S_MODELNAME 12
#define SYSCFG_I_MODELNAME MFG_DATA

#define SYSCFG_N_MACADDRESS "macaddress"
#define SYSCFG_S_MACADDRESS 16
#define SYSCFG_I_MACADDRESS MFG_DATA

#define SYSCFG_N_SERIALNO "serialno"
#define SYSCFG_S_SERIALNO 16
#define SYSCFG_I_SERIALNO MFG_DATA

#define SYSCFG_N_REGIONCODE "region_code"
#define SYSCFG_S_REGIONCODE 10
#define SYSCFG_I_REGIONCODE MFG_DATA

extern int get_syscfg_idx(char *name);
extern void dump_syscfg(void);
extern void generate_syscfg(void);
extern cJSON *dump_syscfg_to_json_object(void);
extern cJSON *make_syscfg_json_object(void);
extern char *make_syscfg_json(void);

#endif /* _SYS_CONFIG_H_ */
