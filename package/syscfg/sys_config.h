#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include "syscfg.h"

#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#define SYSCFG_N_OP_TIME "op_time"
#define SYSCFG_S_OP_TIME 10
#define SYSCFG_I_OP_TIME CFG_DATA

#define SYSCFG_N_CPU0_RESET_REASON "reset0_reason"
#define SYSCFG_S_CPU0_RESET_REASON 5
#define SYSCFG_I_CPU0_RESET_REASON CFG_DATA

#define SYSCFG_N_CPU1_RESET_REASON "reset1_reason"
#define SYSCFG_S_CPU1_RESET_REASON 5
#define SYSCFG_I_CPU1_RESET_REASON CFG_DATA

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

#define SYSCFG_N_MASTER_MAC "master_mac"
#define SYSCFG_S_MASTER_MAC 16
#define SYSCFG_I_MASTER_MAC MFG_DATA

#define SYSCFG_N_HID_MAC "hid_mac"
#define SYSCFG_S_HID_MAC 16
#define SYSCFG_I_HID_MAC MFG_DATA

#define SYSCFG_N_CHILD1_MAC "child1_mac"
#define SYSCFG_S_CHILD1_MAC 16
#define SYSCFG_I_CHILD1_MAC MFG_DATA

#define SYSCFG_N_CHILD2_MAC "child2_mac"
#define SYSCFG_S_CHILD2_MAC 16
#define SYSCFG_I_CHILD2_MAC MFG_DATA

#define SYSCFG_N_CHILD3_MAC "child3_mac"
#define SYSCFG_S_CHILD3_MAC 16
#define SYSCFG_I_CHILD3_MAC MFG_DATA

#define SYSCFG_N_CHILD4_MAC "child4_mac"
#define SYSCFG_S_CHILD4_MAC 16
#define SYSCFG_I_CHILD4_MAC MFG_DATA

#define SYSCFG_N_CHILD5_MAC "child5_mac"
#define SYSCFG_S_CHILD5_MAC 16
#define SYSCFG_I_CHILD5_MAC MFG_DATA

#define SYSCFG_N_CHILD6_MAC "child6_mac"
#define SYSCFG_S_CHILD6_MAC 16
#define SYSCFG_I_CHILD6_MAC MFG_DATA

#define SYSCFG_N_ZONE1_FLOW "zone1_flow"
#define SYSCFG_S_ZONE1_FLOW 12
#define SYSCFG_I_ZONE1_FLOW CFG_DATA

#define SYSCFG_N_ZONE2_FLOW "zone2_flow"
#define SYSCFG_S_ZONE2_FLOW 12
#define SYSCFG_I_ZONE2_FLOW CFG_DATA

#define SYSCFG_N_ZONE3_FLOW "zone3_flow"
#define SYSCFG_S_ZONE3_FLOW 12
#define SYSCFG_I_ZONE3_FLOW CFG_DATA

#define SYSCFG_N_ZONE4_FLOW "zone4_flow"
#define SYSCFG_S_ZONE4_FLOW 12
#define SYSCFG_I_ZONE4_FLOW CFG_DATA

#define SYSCFG_N_ZONE5_FLOW "zone5_flow"
#define SYSCFG_S_ZONE5_FLOW 12
#define SYSCFG_I_ZONE5_FLOW CFG_DATA

#define SYSCFG_N_ZONE6_FLOW "zone6_flow"
#define SYSCFG_S_ZONE6_FLOW 12
#define SYSCFG_I_ZONE6_FLOW CFG_DATA

int get_syscfg_idx(char *name);
void dump_syscfg(void);
void generate_syscfg(void);
cJSON *dump_syscfg_to_json_object(void);
cJSON *make_syscfg_json_object(void);
char *make_syscfg_json(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_CONFIG_H_ */
