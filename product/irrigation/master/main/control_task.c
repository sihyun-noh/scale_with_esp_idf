#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "config.h"
#include "log.h"
#include "mqtt_task.h"
#include "sys_status.h"
#include "syscfg.h"
#include "actuator.h"
#include "espnow.h"
#include "time.h"
#include "main.h"
#include "comm_packet.h"
#include "battery_task.h"

#define RESP_THRES 20
#define NUMBER_CHILD 6
#define TOTAL_DEVICES 7
#define SYNC_TIME_BUFF 5

#define CMD_MSG_QUEUE_LEN 16

#define DEBUG_TEST 1  // for Test

#define AM_TIME_FLOW_START 5   // AM 5시
#define AM_TIME_FLOW_END 9     // AM 9시
#define PM_TIME_FLOW_START 17  // PM 5시
#define PM_TIME_FLOW_END 21    // PM 9시

#define IRRIGATION_SIMULATOR 1

static const char* TAG = "control_task";

static TaskHandle_t control_handle = NULL;
static TaskHandle_t cmd_handle = NULL;
static QueueHandle_t cmd_msg_queue = NULL;

extern int get_water_flow_liters(void);
extern void reset_water_flow_liters(void);

typedef enum {
  CHECK_ADDR = 0,
  SYNC_TIME,
  CHECK_TIME,
  WAIT_STATE,
  CHECK_SCEHDULE,
  CHECK_FLOW,
  CHILD_VALVE_ON,
  CHILD_VALVE_OFF,
  COMPLETE,
  SLEEP,
  ERROR
} condtrol_status_t;

condtrol_status_t controlStatus = CHECK_ADDR;

message_type_t sendCmd;  // send cmd name
bool sendMessageFlag;    // cmd send true/false
int respTimeCnt;         // response
int receivedId;          // received device Id
int respBroadCast[TOTAL_DEVICES];
int retryCntMsg[TOTAL_DEVICES];

bool setConfig;  // config setting 여부
bool flowStart;  // 관수 시작 flag
bool sendChildSleep;

time_t flowStartTime;                // 관수 시작 설정 시간
int zoneFlowCnt;                     // 관수 설정된 zone 갯수
int flowOrder[NUMBER_CHILD];         // 관수 순서
int flowSettingValue[NUMBER_CHILD];  // 관수 설정 값
int zoneBattery[TOTAL_DEVICES];      // 0: master, 1~6: child 배터리 잔량

int flowDoneCnt;       // zone 변 관수 완료 카운트
int respChildCnt;      // child response 수신 횟수
int syncTimeBuffCnt;   // Time sync buffer check count
int totalNumberChild;  // child device number

uint64_t remainSleepTime;  // sleep 시간

static bool _send_msg_event(irrigation_message_t* msg) {
  return cmd_msg_queue && (xQueueSend(cmd_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

static bool _get_msg_event(irrigation_message_t* msg) {
  return cmd_msg_queue && (xQueueReceive(cmd_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

void send_msg_to_cmd_task(void* msg, size_t msg_len) {
  irrigation_message_t message = { 0 };

  if (msg && (msg_len == sizeof(irrigation_message_t))) {
    memcpy(&message, msg, msg_len);
    if (!_send_msg_event(&message)) {
      LOGW(TAG, "Failed to send recv message event!!!");
    }
  }
}

void init_variable(void) {
  sendCmd = NONE;
  sendMessageFlag = false;
  respTimeCnt = 0;
  receivedId = -1;
  memset(&respBroadCast, 0x00, sizeof(respBroadCast));
  memset(&retryCntMsg, 0x00, sizeof(retryCntMsg));

  setConfig = false;
  flowStart = false;
  sendChildSleep = false;
  memset(&flowStartTime, 0x00, sizeof(flowStartTime));
  zoneFlowCnt = 0;
  memset(&flowOrder, 0x00, sizeof(flowOrder));
  memset(&flowSettingValue, 0x00, sizeof(flowSettingValue));
  memset(&zoneBattery, 0x00, sizeof(zoneBattery));
  flowDoneCnt = 0;
  respChildCnt = 0;
  syncTimeBuffCnt = 0;
  totalNumberChild = 0;
  remainSleepTime = 0;
}

void set_control_status(condtrol_status_t value) {
  controlStatus = value;
  if (value != WAIT_STATE) {
    respTimeCnt = 0;
  }
}

void start_flow(void) {
  // sent message to zone valve on
  reset_water_flow_liters();
  pump_on();
}

void stop_flow(void) {
  pump_off();
  // sent message to zone valve off
}

int get_flow_value() {
  int value = 0;
  // 유량계를 통해 값을 읽어온다...
  value = get_water_flow_liters();

  return value;
}

bool get_addr(int zone, uint8_t* mac_addr) {
  bool valid_mac = false;

  if (zone == ALL_DEV) {
    // Broadcasting address
    memset(mac_addr, 0xFF, sizeof(uint8_t) * MAC_ADDR_LEN);
    valid_mac = true;
  } else {
    valid_mac = get_mac_address(zone, mac_addr);
  }
  return valid_mac;
}

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

// 입력 mac addr 이 현 set mac addr 인지 확인 함수
int check_address_matching_current_set(const uint8_t* mac) {
  for (int i = 0; i < TOTAL_DEVICES; i++) {
    uint8_t compareAddr[MAC_ADDR_LEN] = { 0 };

    if (get_addr(i, compareAddr)) {
      LOGI(TAG, "======== zone id [%d] ========", i);
      LOG_BUFFER_HEX(TAG, compareAddr, MAC_ADDR_LEN);

      if (memcmp(mac, compareAddr, sizeof(compareAddr)) == 0) {
        LOGI(TAG, "addr matched zone ID : %d ", i);
        return i;
      }
    }
  }
  return -1;
}

// if return 0 -> wake up 시간대
// return value -> sleep 시간까지 남은 시간, 단위 seconds
uint64_t check_sleep_time(void) {
  time_t currTime = get_current_time();
  struct tm timeinfo = { 0 };
  localtime_r(&currTime, &timeinfo);

  if (timeinfo.tm_year < 120) {
    LOGI(TAG, "Time set not yet ");
    return 0;
  }

  // 시간 sleep time / wake time 비교...
  // 오전 5시 ~ 9시 wake up...
  // 17시 ~ 21시 wake up...
  // 버퍼를 두기 위해 wake up 시간 대비 30분 전까지만 sleep
  if (timeinfo.tm_hour < AM_TIME_FLOW_START) {
    if ((timeinfo.tm_hour == (AM_TIME_FLOW_START - 1)) && timeinfo.tm_min >= 30) {
      return 0;
    }
    return (uint64_t)((((AM_TIME_FLOW_START - 1) - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60));
  } else if (timeinfo.tm_hour >= AM_TIME_FLOW_END && timeinfo.tm_hour < PM_TIME_FLOW_START) {
    if ((timeinfo.tm_hour == (PM_TIME_FLOW_START - 1)) && timeinfo.tm_min >= 30) {
      return 0;
    }
    return (uint64_t)(((PM_TIME_FLOW_START - 1 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60));
  } else if (timeinfo.tm_hour >= PM_TIME_FLOW_END) {
    return (uint64_t)((((23 + AM_TIME_FLOW_START) - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60));
  } else {
    return 0;
  }
}

void send_mqtt_data(message_type_t sender, message_type_t receiver) {
  mqtt_response_t mqtt_resp = { 0 };

  if (sender == START_FLOW && receiver == START_FLOW) {
    // Publish status data to the mqtt broker.
    mqtt_resp.resp = RESP_FLOW_START;
    mqtt_resp.payload.dev_stat.deviceId = flowOrder[flowDoneCnt];
  } else if (sender == ZONE_COMPLETE && receiver == ZONE_COMPLETE) {
    mqtt_resp.resp = RESP_FLOW_DONE;
    mqtt_resp.payload.dev_stat.deviceId = flowOrder[flowDoneCnt];
    mqtt_resp.payload.dev_stat.flow_value = get_flow_value();
  } else if (sender == RESPONSE && receiver == FORCE_STOP) {
    mqtt_resp.resp = RESP_FORCE_STOP;
    mqtt_resp.payload.dev_stat.deviceId = flowOrder[flowDoneCnt];
    mqtt_resp.payload.dev_stat.flow_value = get_flow_value();
  }

  send_msg_to_mqtt_task(MQTT_PUB_DATA_ID, &mqtt_resp, sizeof(mqtt_response_t));
}

bool send_esp_data(message_type_t sender, message_type_t receiver, int id) {
  irrigation_message_t send_message = { 0 };
  uint8_t zoneAddr[MAC_ADDR_LEN] = { 0 };

  if (!get_addr(id, zoneAddr)) {
    return false;
  }

  receivedId = id;

  send_message.sender_type = sender;
  send_message.receive_type = receiver;
  send_message.current_time = get_current_time();

  payload_t* payload = &send_message.payload;

  switch (sender) {
    case TIME_SYNC: {
      respBroadCast[0] = 1;
    } break;
    case START_FLOW: {
      payload->dev_stat.deviceId = flowOrder[flowDoneCnt];
      LOGI(TAG, "start flow zone : %d ", payload->dev_stat.deviceId);
    } break;
    case ZONE_COMPLETE: {
      payload->dev_stat.deviceId = flowOrder[flowDoneCnt];
      payload->dev_stat.flow_value = get_flow_value();
      LOGI(TAG, "flow complete zone : %d, flow = %d", payload->dev_stat.deviceId, payload->dev_stat.flow_value);
    } break;
    case BATTERY_LEVEL: {
      memcpy(payload->dev_stat.battery_level, zoneBattery, sizeof(payload->dev_stat.battery_level));
    } break;
    case DEVICE_ERROR: {
      for (int i = 0; i < NUMBER_CHILD; i++) {
        if (retryCntMsg[i + 1] > 3) {
          payload->dev_stat.child_status[i] = 1;
        }
      }
    } break;
    case SET_SLEEP: {
      payload->remain_time_sleep = remainSleepTime;
    } break;
    case RESPONSE: {
      if (receiver == FORCE_STOP) {
        payload->dev_stat.deviceId = flowOrder[flowDoneCnt];
        payload->dev_stat.flow_value = get_flow_value();
      }
    } break;
    default: break;
  }

  LOGI(TAG, "============== ZoneAddr =================");
  LOG_BUFFER_HEXDUMP(TAG, zoneAddr, sizeof(zoneAddr), LOG_INFO);

  LOGI(TAG, "============== Message ==================");
  // LOG_BUFFER_HEXDUMP(TAG, &send_message, sizeof(irrigation_message_t), LOG_INFO);
  LOGI(TAG, "sender : [%d]", sender);

  if (!espnow_send_data(zoneAddr, (uint8_t*)&send_message, sizeof(irrigation_message_t))) {
    LOGE(TAG, "Failed to esp send data!!!");
    espnow_send_data(zoneAddr, (uint8_t*)&send_message, sizeof(irrigation_message_t));
  } else {
    LOGI(TAG, "Success to esp send data!!!");
  }

  sendCmd = sender;
  sendMessageFlag = true;

  return true;
}

void check_response(irrigation_message_t* msg) {
  if ((sendCmd == msg->receive_type) && (msg->resp == SUCCESS)) {
    sendMessageFlag = false;
    sendCmd = NONE;
  }

  switch (msg->receive_type) {
    case START_FLOW: {
      set_control_status(CHECK_FLOW);
    } break;

    case ZONE_COMPLETE: {
      flowDoneCnt++;
      set_control_status(CHECK_SCEHDULE);
    } break;

    case DEVICE_ERROR: {
      respChildCnt = 0;
      set_control_status(ERROR);
    } break;

    case BATTERY_LEVEL: {
      set_control_status(CHECK_SCEHDULE);
    } break;

    case ALL_COMPLETE: {
      init_variable();
      set_control_status(CHECK_SCEHDULE);
    } break;

    case SET_VALVE_ON: {
      if (msg->resp == SUCCESS) {
        start_flow();
        // 관수 시작을 HID 에 전달
        send_esp_data(START_FLOW, START_FLOW, HID_DEV);

        LOGI(TAG, "RECEIVE VALVE ON RESPONSE CHILD-%d", flowOrder[flowDoneCnt]);

        send_mqtt_data(START_FLOW, START_FLOW);

        set_control_status(WAIT_STATE);
      }
    } break;

    case SET_VALVE_OFF: {
      if (msg->resp == SUCCESS) {
        // 관수 완료를 HID 에 전달
        send_esp_data(ZONE_COMPLETE, ZONE_COMPLETE, HID_DEV);

        LOGI(TAG, "RECEIVE VALVE OFF RESPONSE CHILD-%d", flowOrder[flowDoneCnt]);

        send_mqtt_data(ZONE_COMPLETE, ZONE_COMPLETE);

        set_control_status(WAIT_STATE);
      }
    } break;

    case TIME_SYNC: {
      device_status_t* dev_stat = (device_status_t*)&msg->payload.dev_stat;
      zoneBattery[dev_stat->deviceId] = dev_stat->battery_level[dev_stat->deviceId];
      LOGI(TAG, "ID: %D, Battery level: %d", dev_stat->deviceId, dev_stat->battery_level[dev_stat->deviceId]);
      respChildCnt++;
      sendMessageFlag = true;
      sendCmd = msg->receive_type;

      respBroadCast[dev_stat->deviceId] = 1;

      if (respChildCnt >= totalNumberChild) {
        memset(&respBroadCast, 0x00, sizeof(respBroadCast));
        sendMessageFlag = false;
        sendCmd = NONE;
        zoneBattery[0] = read_battery_percentage();
        LOGI(TAG, "Battery Level, Master: %d, Child : %d, %d, %d, %d, %d, %d ", zoneBattery[0], zoneBattery[1],
             zoneBattery[2], zoneBattery[3], zoneBattery[4], zoneBattery[5], zoneBattery[6]);

        send_esp_data(BATTERY_LEVEL, BATTERY_LEVEL, HID_DEV);

        respChildCnt = 0;
        set_control_status(WAIT_STATE);
      }
    } break;

    case SET_SLEEP: {
      device_status_t* dev_stat = (device_status_t*)&msg->payload.dev_stat;
      sendMessageFlag = true;
      sendCmd = msg->receive_type;
      respChildCnt++;

      LOGI(TAG, "resp CNT Child : %d and total : %d for Sleep Command", respChildCnt, totalNumberChild);

      respBroadCast[dev_stat->deviceId] = 1;

      if (respChildCnt >= totalNumberChild) {
        LOGI(TAG, "Send Sleep Command to the HID device and entering sleep mode on main");
        memset(&respBroadCast, 0x00, sizeof(respBroadCast));
        remainSleepTime -= 20;
        send_esp_data(SET_SLEEP, SET_SLEEP, HID_DEV);
        respChildCnt = 0;

        set_control_status(SLEEP);
      }
    } break;

    case FORCE_STOP: {
      // Send force stop response to the hid device when master received reposne of force stop from child node.
      send_esp_data(RESPONSE, FORCE_STOP, HID_DEV);

      LOGI(TAG, "Send Force STOP MQTT command!!!");

      send_mqtt_data(RESPONSE, FORCE_STOP);

      init_variable();
      set_control_status(CHECK_SCEHDULE);
    } break;

    default: break;
  }
}

void show_config_debug(void) {
#if defined(DEBUG_TEST)
  struct tm timeinfo = { 0 };
  localtime_r(&flowStartTime, &timeinfo);

  LOGI(TAG, "flow start time %02d : %02d ", timeinfo.tm_hour, timeinfo.tm_min);
  LOGI(TAG, "Zone flow cnt : %d ", zoneFlowCnt);

  for (int i = 0; i < zoneFlowCnt; i++) {
    LOGI(TAG, "flow zone: %d, flow rate: %d ", flowOrder[i], flowSettingValue[i]);
  }
#endif
}

void device_time_sync(void) {
  send_esp_data(TIME_SYNC, TIME_SYNC, ALL_DEV);

  LOGI(TAG, "SEND HID/CHILD TIME SYNC !!");
  set_control_status(WAIT_STATE);
}

void check_peer_address(void) {
  uint8_t peer_mac_address[TOTAL_DEVICES][MAC_ADDR_LEN] = { { 0 } };
  totalNumberChild = 0;
  // 각 device mac addr 존재 여부 판단... 확인 완료 시 check mode 단계..
  if (espnow_add_peers(MASTER_DEVICE)) {
    LOGI(TAG, "Success to add hid, child addr to peer list");
  } else {
    LOGI(TAG, "Failed to add hid, child addr to peer list");
  }

  if (!get_mac_address(HID_DEV, peer_mac_address[HID_DEV])) {
    LOGW(TAG, "Failed to get HID mac address");
  }

  if (get_mac_address(CHILD_1, peer_mac_address[CHILD_1])) {
    totalNumberChild++;
  }

  if (get_mac_address(CHILD_2, peer_mac_address[CHILD_2])) {
    totalNumberChild++;
  }

  if (get_mac_address(CHILD_3, peer_mac_address[CHILD_3])) {
    totalNumberChild++;
  }

  if (get_mac_address(CHILD_4, peer_mac_address[CHILD_4])) {
    totalNumberChild++;
  }

  if (get_mac_address(CHILD_5, peer_mac_address[CHILD_5])) {
    totalNumberChild++;
  }

  if (get_mac_address(CHILD_6, peer_mac_address[CHILD_6])) {
    totalNumberChild++;
  }

  LOGI(TAG, "Number of Child device : %d", totalNumberChild);

  LOG_BUFFER_HEXDUMP(TAG, peer_mac_address, sizeof(peer_mac_address), LOG_INFO);

  set_control_status(CHECK_TIME);
  LOGI(TAG, "ADDR CHECK DONE !!");
}

/**
 * ! ESPNOW sending or receiving callback function is called in WiFi task.
 * ! Users should not do lengthy operations from this task. Instead, post
 * ! necessary data to a queue and handle it from a lower priority task.
 */
void on_data_recv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  LOGI(TAG, "===== recv mac address ========");
  LOG_BUFFER_HEX(TAG, mac, MAC_ADDR_LEN);

  int msgFromId = check_address_matching_current_set(mac);
  if (msgFromId == (-1)) {
    LOGI(TAG, "Receive data from other SET");
    return;
  }

  retryCntMsg[msgFromId] = 0;

  LOGI(TAG, "Receive Data from Other devices(HID and Any Child) : %d", msgFromId);

  send_msg_to_cmd_task((void*)incomingData, len);
}

void on_data_sent_cb(const uint8_t* macAddr, esp_now_send_status_t status) {
  LOGI(TAG, "Delivery status : %d ", status);
}

void check_set_local_time(void) {
  time_t currTime = get_current_time();
  struct tm timeinfo = { 0 };
  localtime_r(&currTime, &timeinfo);

  if (timeinfo.tm_year < 120) {
    LOGI(TAG, "Time set not yet ");
  } else {
    set_control_status(SYNC_TIME);
  }
}

bool is_flow_start(void) {
  time_t currentTime;
  struct tm currentTimeinfo = { 0 };
  struct tm flowTimeinfo = { 0 };

  time(&currentTime);
  localtime_r(&currentTime, &currentTimeinfo);
  localtime_r(&flowStartTime, &flowTimeinfo);

  if ((currentTimeinfo.tm_hour == flowTimeinfo.tm_hour) && (currentTimeinfo.tm_min == flowTimeinfo.tm_min)) {
    LOGI(TAG, "It's time for Flow Start!! ");
    return true;
  }
  return false;
}

void check_schedule(void) {
  // 현재 시간과 관수 스케쥴 시간을 비교 하는 단계.
  // 스케쥴 시간에 도달하면 valve on 과 함께 관수 시작
  if (setConfig) {  // 설정된 config가 있을 경우에 config 에 따라 스케줄 관리 시작
    if (!flowStart) {
      if (is_flow_start()) {
        set_control_status(CHILD_VALVE_ON);
        LOGI(TAG, "START FLOW!!");
        flowStart = true;
      }
    } else {
      if (flowDoneCnt >= zoneFlowCnt) {
        set_control_status(COMPLETE);
      } else {
        LOGI(TAG, "START NEXT CHILD FLOW!!");
        set_control_status(CHILD_VALVE_ON);
      }
    }
  } else {
    remainSleepTime = check_sleep_time();
    // response Time 고려 30초 이내일 경우 sleep 안 들어가고 그냥 wating 모드로 ..
    if (remainSleepTime > 30) {
      LOGI(TAG, "SEND DEEP SLEEP MSG, SleepTime : %llus ", remainSleepTime);
      if (!sendChildSleep) {
        memset(&respBroadCast, 0x00, sizeof(respBroadCast));
        memset(&retryCntMsg, 0x00, sizeof(retryCntMsg));
        sendChildSleep = true;
        for (int i = 1; i <= 6; i++) {
          send_esp_data(SET_SLEEP, SET_SLEEP, (device_type_t)i);
        }
        set_control_status(WAIT_STATE);
      } else {
        remainSleepTime -= 20;
        send_esp_data(SET_SLEEP, SET_SLEEP, HID_DEV);
        set_control_status(SLEEP);
      }
    } else {
      LOGD(TAG, "WAIT SET CONFIG ");
    }
  }
}

void check_retry_cmd(void) {
  if (retryCntMsg[0] > 3) {
    LOGI(TAG, "HID Device ERROR !! ");
    set_control_status(ERROR);
    retryCntMsg[0] = 0;
    if (sendCmd == ZONE_COMPLETE) {
      flowDoneCnt++;
    }
  }

  int childErrorCnt = 0;
  for (int i = 1; i < TOTAL_DEVICES; i++) {
    if (retryCntMsg[i] > 3) {
      childErrorCnt++;
      LOGI(TAG, "Zone-%d Device ERROR !!", i);
    }
  }

  if (childErrorCnt > 0) {
    send_esp_data(DEVICE_ERROR, DEVICE_ERROR, HID_DEV);
    set_control_status(WAIT_STATE);
  }
}

void retry_send_cmd(int rcvId) {
  if (rcvId > NUMBER_CHILD) {
    // broadcasting 후에 못 받은 것들 체크....
    for (int i = 1; i < TOTAL_DEVICES; i++) {
      if (respBroadCast[i] == 0) {
        if (send_esp_data(sendCmd, sendCmd, i)) {
          retryCntMsg[i] += 1;
        }
      }
      receivedId = TOTAL_DEVICES;
    }
  } else {
    send_esp_data(sendCmd, sendCmd, rcvId);
    retryCntMsg[rcvId] += 1;
  }
  respTimeCnt = 0;
}

void check_cmd_response(void) {
  if (sendMessageFlag && (sendCmd != NONE)) {
    // 카운트... 10초 이상이면.. 다시 retry send...
    respTimeCnt++;
    if (respTimeCnt > RESP_THRES) {
      LOGI(TAG, "Retry CMD to %d ", receivedId);
      retry_send_cmd(receivedId);
      check_retry_cmd();
    }
  } else {
    respTimeCnt = 0;
  }
}

void cmd_msg_handler(irrigation_message_t* message) {
  // LOG_BUFFER_HEXDUMP(TAG, incomingData, len, LOG_INFO);

  // master 에서는 아래 세가지 경우만 필요.
  // HID 로 부터 수신하는 config 설정 값
  // Child 로 전달한 valve on / off 에 대한 response
  // Child 의 배터리 잔량 값.
  switch (message->sender_type) {
    case SET_CONFIG: {
      config_value_t* config = (config_value_t*)&message->payload.config;
      flowStartTime = config->start_time;
      zoneFlowCnt = config->zone_cnt;
      memcpy(flowOrder, config->zones, sizeof(flowOrder));
      memcpy(flowSettingValue, config->flow_rate, sizeof(flowSettingValue));
      flowDoneCnt = 0;
      setConfig = true;
      show_config_debug();

      send_esp_data(RESPONSE, SET_CONFIG, HID_DEV);

      set_control_status(CHECK_SCEHDULE);
      LOGI(TAG, "RECEIVE & SET CONFIG from HID");
    } break;

    case RESPONSE: {
      check_response(message);
    } break;

    case FORCE_STOP: {
      LOGI(TAG, "FORCE STOP !!");

      if (flowStart) {
        stop_flow();
        send_esp_data(FORCE_STOP, FORCE_STOP, flowOrder[flowDoneCnt]);
        set_control_status(WAIT_STATE);
        flowStart = false;
      } else {
        send_esp_data(RESPONSE, FORCE_STOP, HID_DEV);
        init_variable();
        set_control_status(CHECK_SCEHDULE);
      }
    } break;

    case REQ_TIME_SYNC: {
      send_esp_data(TIME_SYNC, TIME_SYNC, ALL_DEV);

      LOGI(TAG, "SEND TIME SYNC / REQ_TIME_SYNC!!");
      set_control_status(WAIT_STATE);
    } break;

    case UPDATE_DEVICE_ADDR: {
      send_esp_data(RESPONSE, UPDATE_DEVICE_ADDR, HID_DEV);

      device_manage_t* dev_manage = (device_manage_t*)&message->payload.dev_manage;
      int cnt = dev_manage->update_dev_cnt;
      device_addr_t* update_dev_addr = dev_manage->update_dev_addr;
      LOGI(TAG, "update device addr cnt = %d", cnt);
      // Remove all peer address from espnow
      espnow_remove_peers(MASTER_DEVICE);
      // Save the updated peer address to the syscfg (Only child address should be updated)
      for (int i = 0; i < cnt; i++) {
        LOGI(TAG, "device type = %d, mac_addr = %s", update_dev_addr[i].device_type, update_dev_addr[i].mac_addr);
        if (update_dev_addr[i].device_type != MAIN_DEV) {
          set_mac_address(update_dev_addr[i].device_type, update_dev_addr[i].mac_addr);
        }
      }
      // Add updated peer address raed from syscfg to the espnow
      check_peer_address();
    } break;

    default: break;
  }
}

static void cmd_task(void* pvParameters) {
  irrigation_message_t msg;

  for (;;) {
    memset(&msg, 0x00, sizeof(irrigation_message_t));
    if (_get_msg_event(&msg)) {
      cmd_msg_handler(&msg);
    } else {
      LOGE(TAG, "Failed to receive message from queue");
      vTaskDelay(1000);
      continue;
    }
    vTaskDelay(500);
  }
  vTaskDelete(NULL);
  cmd_handle = NULL;
}

static void control_task(void* pvParameters) {
  for (;;) {
    switch (controlStatus) {
      case CHECK_ADDR: {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        check_peer_address();
      } break;

      case CHECK_TIME: {
        check_set_local_time();
      } break;

      case SYNC_TIME: {
        // master 시간을 기준으로 message 생성하여 broadcasting
        // 각 device 에서는 전달받은 시간 값으로 time sync
        // 다른 device 별 rtc 차이로 깨어 나는 시간 차이를 위해 시간 버퍼 추가.
        // 5 분으로 시간 버퍼 적용 --> 테스트 진행 후 값 튜닝 필요
        // HID 에서는 wake up 후 10분 안에 time sync 가 없을 경우 master 로 다시 time sync req
        syncTimeBuffCnt++;
        if (syncTimeBuffCnt > (60 * SYNC_TIME_BUFF)) {
          device_time_sync();
          syncTimeBuffCnt = 0;
        }
      } break;

      case WAIT_STATE: {
        // 대기 상태
        check_cmd_response();
      } break;

      case CHECK_SCEHDULE: {
        check_schedule();
      } break;

      case CHECK_FLOW: {
        // 유량 체크 후 HID 에 유량 값 전달...
        // 일정 값마다...또는 일정 시간마다 아래 메세지 전달... --> 추가 논의 필요.
        /*
        memset(&send_message, 0x00, sizeof(send_message));
        send_message.sender_type = FLOW_STATUS;
        send_message.deviceId = flowOrder[flowDoneCnt]; //  valve 를 on 할 child number
        send_message.flow_value = get_flow_value();
        send_message.current_time = get_current_time();
        espnow_send_data(macAddress[0][], (uint8_t *) &send_message, sizeof(send_message));
        */

        // 유량 체크, 설정값과 차이가 20리터 이내로 들어올 경우 valve off 하도록... -> 테스트 후 값 변경 필요.
        int currentFlowValue = get_flow_value();
        if (currentFlowValue >= flowSettingValue[flowDoneCnt]) {
          LOGI(TAG, "Current Flow is %d. close valve ", currentFlowValue);
          stop_flow();  // main pump off
          set_control_status(CHILD_VALVE_OFF);
        }
      } break;

      case CHILD_VALVE_ON: {
        // 관수 스케줄에 따라 pump / zone pump on, off 컨트롤
        // 관수 시작 시 : zone valve on -> pump on
        if (send_esp_data(SET_VALVE_ON, SET_VALVE_ON, flowOrder[flowDoneCnt])) {
          LOGI(TAG, "CHILD -%d VALVE ON !!", flowOrder[flowDoneCnt]);
          set_control_status(WAIT_STATE);
        } else {
          LOGI(TAG, "CHILD -%d is unavailable !!", flowOrder[flowDoneCnt]);
          flowDoneCnt++;
          set_control_status(CHECK_SCEHDULE);
        }
      } break;

      case CHILD_VALVE_OFF: {
        // 관수 스케줄에 따라 pump / zone pump on, off 컨트롤
        // 관수 중지 시 : pump off -> zone valve off
        send_esp_data(SET_VALVE_OFF, SET_VALVE_OFF, flowOrder[flowDoneCnt]);

        LOGI(TAG, "CHILD - %d VALVE OFF !!", flowOrder[flowDoneCnt]);
        set_control_status(WAIT_STATE);
      } break;

      case COMPLETE: {
        // 관수 스케줄대로 모든 관수가 종료된 상태.
        // 추가 관수 여부 확인..--> wait config 모드로
        send_esp_data(ALL_COMPLETE, ALL_COMPLETE, HID_DEV);
        LOGI(TAG, "ALL CHILD FLOW COMPLETE!!");
        set_control_status(WAIT_STATE);
      } break;

      case SLEEP: {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        LOGI(TAG, "start master sleep");
        sleep_timer_wakeup(remainSleepTime);
      } break;

      case ERROR: {
        LOGD(TAG, "No Response from sets !!, Fix SET Devices state & RESET System !!");
        set_control_status(CHECK_SCEHDULE);
      } break;

      default: break;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void create_control_task(void) {
  uint16_t control_stack_size = 8192;
  uint16_t cmd_stack_size = 4096;
  UBaseType_t control_task_priority = tskIDLE_PRIORITY + 5;
  UBaseType_t cmd_task_priority = tskIDLE_PRIORITY + 3;

  if (control_handle) {
    LOGI(TAG, "control task is alreay created");
    return;
  }

  // create queue
  cmd_msg_queue = xQueueCreate(CMD_MSG_QUEUE_LEN, sizeof(irrigation_message_t));
  if (cmd_msg_queue == NULL) {
    return;
  }

  // create control task
  init_variable();
  xTaskCreate(control_task, "CONTROL", control_stack_size, NULL, control_task_priority, &control_handle);

  // create espnow command handler task
  xTaskCreate(cmd_task, "COMMAND", cmd_stack_size, NULL, cmd_task_priority, &cmd_handle);
}
