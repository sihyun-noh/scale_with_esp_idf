#ifndef _MQTT_TOPIC_H_
#define _MQTT_TOPIC_H_

// Topic name

/* Value topic name for sensors */
#define TEMPERATURE_PUB_SUB_TOPIC "value/%s/temperature"
#define HUMIDITY_PUB_SUB_TOPIC "value/%s/humidity"
#define CO2_PUB_SUB_TOPIC "value/%s/co2"
#define EC_PUB_SUB_TOPIC "value/%s/ec"
#define SOLAR_PUB_SUB_TOPIC "value/%s/solar"
#define PH_PUB_SUB_TOPIC "value/%s/ph"
#define BULK_EC_PUB_SUB_TOPIC "value/%s/bulkec"
#define WIND_DIRECTION_PUB_SUB_TOPIC "value/%s/wind_direction"
#define WIND_SPEED_PUB_SUB_TOPIC "value/%s/wind_speed"

/* Command Request & Response */
#define CMD_REQUEST_TOPIC "cmd/%s/req"
#define CMD_RESPONSE_TOPIC "cmd/%s/resp"

// Request & Response mqtt payload key name
#define REQRES_K_URL "url"
#define REQRES_K_SSID "SSID"
#define REQRES_K_PASSWORD "Password"

#define REQRES_K_DEVICEID "id"
#define REQRES_K_FWVERSION "fw_version"
#define REQRES_K_FARMNETWORK "farm_network"
#define REQRES_K_FARMSSID "ssid"
#define REQRES_K_CHANNEL "channel"
#define REQRES_K_IPADDRESS "ip_address"
#define REQRES_K_RSSI "rssi"
#define REQRES_K_SENDINTERVAL "send_interval"
#define REQRES_K_POWERMODE "power"
#define REQRES_K_UPTIME "uptime"
#define REQRES_K_FREEMEM "free_mem"
#define REQRES_K_RECONNECT "reconnect"
#define REQRES_K_TIMESTAMP "timestamp"
#define REQRES_K_STATE "state"
#define REQRES_K_VALUE "value"
#define REQRES_K_BATTERY "battery"
#define REQRES_K_SERVER "Server"
#define REQRES_K_ACTION "action"
#define REQRES_K_RESULT "result"

#define REQRES_K_TYPE "type"
// Request & Response mqtt payload value name
#define REQRES_V_DEVINFO "devinfo"
#define REQRES_V_FWUPDATE "fw_update"
#define REQRES_V_UPDATE "update"
#define REQRES_V_RESET "reset"
#define REQRES_V_FACTORY_RESET "factory_reset"
#define REQRES_V_WIFI_CHANGE "wifi_change"
#define REQRES_V_SERVER_CHANGE "server_change"
#define REQRES_V_SEARCH "search"
#define REQRES_V_SPIFFS_FORMAT "spiffs_format"
#define REQRES_V_SYSCFG "syscfg"
#define REQRES_V_SYSCFG_SET "set"
#define REQRES_V_SYSCFG_GET "get"
#define REQRES_V_SYSCFG_SHOW "show"
#define REQRES_V_SYSCFG_OK "ok"
#define REQRES_V_ERR_SYSCFG_FAIL "fail"
#define REQRES_V_ERR_SYSCFG_NONE "none"

#endif /* _MQTT_TOPIC_H_ */
