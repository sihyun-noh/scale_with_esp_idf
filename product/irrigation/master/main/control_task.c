#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"
#include "log.h"
#include "sys_status.h"
#include "syscfg.h"
#include "actuator.h"
#include "espnow.h"
#include "time.h"

static const char* TAG = "control_task";
static TaskHandle_t control_handle = NULL;

extern int get_water_flow_liters(void);

typedef enum {
  SET_CONFIG = 0,
  TIME_SYNC,
  START_FLOW,
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
  time_t start_time;  // time structure 를 어떻게 가져갈 것인지 확인.
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
  int remain_time_sleep;
  time_t current_time;  // 논의 필요.
} irrigation_message_t;

// TODO - 윤형주 코멘트 -
// REGISTER_CB 상태는 필요 없어 보입니다.
// 각 자식 디바이스와 HID 디바이스의 맥 주소와 이를 peer list 에 추가 하는 작업과
// recv/send 콜백을 등록 하는 작업은 espnow_start() 와 espnow_add_peers() 에서 수행합니다
typedef enum {
  CHECK_ADDR = 0,
  // REGISTER_CB,
  SYNC_TIME,
  WAIT_STATE,
  CHECK_SCEHDULE,
  CHECK_FLOW,
  CHILD_VALVE_ON,
  CHILD_VALVE_OFF,
  COMPLETE,
  ERROR
} condtrol_status_t;

condtrol_status_t controlStatus = CHECK_ADDR;

bool setConfig;
bool flowStart;
time_t flowStartTime;
int zoneFlowCnt;
int flowOrder[6];
int flowSettingValue;

int flowDoneCnt;
extern uint8_t macAddress[7][6];  // 0 = HID, 1~6 = child 1~6

void init_variable(void) {
  setConfig = false;
  flowStart = false;
  memset(&flowStartTime, 0x00, sizeof(flowStartTime));
  zoneFlowCnt = 0;
  memset(&flowOrder, 0x00, sizeof(flowOrder));
  flowSettingValue = 0;
  flowDoneCnt = 0;
}

void set_control_status(condtrol_status_t value) {
  controlStatus = value;
}

void start_flow(void) {
  // sent message to zone valve on
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

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

void on_data_recv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  irrigation_message_t recv_message;
  memcpy(&recv_message, incomingData, sizeof(recv_message));

  LOGI(TAG, "Receive Data from Other devices(HID and Any Child)");
  LOG_BUFFER_HEXDUMP(TAG, incomingData, len, LOG_INFO);

  if (len > 0) {
    // master 에서는 아래 세가지 경우만 필요.
    // HID 로 부터 수신하는 config 설정 값
    // Child 로 전달한 valve on / off 에 대한 response
    // Child 의 배터리 잔량 값.
    switch (recv_message.sender_type) {
      case SET_CONFIG:
        flowStartTime = recv_message.config.start_time;
        zoneFlowCnt = recv_message.config.zone_cnt;
        memcpy(flowOrder, recv_message.config.zones, sizeof(flowOrder));
        flowSettingValue = recv_message.config.flow_rate;
        flowDoneCnt = 0;
        setConfig = true;
        set_control_status(CHECK_SCEHDULE);
        LOGI(TAG, "RECEIVE & SET CONFIG from HID");
        break;

      case RESPONSE:
        if (recv_message.receive_type == SET_VALVE_ON) {
          if (recv_message.resp == SUCCESS) {
            start_flow();
            // 관수 시작을 HID 에 전달
            irrigation_message_t send_message;
            memset(&send_message, 0x00, sizeof(send_message));
            send_message.sender_type = START_FLOW;
            send_message.deviceId = flowOrder[flowDoneCnt];  //  valve 를 on 할 child number
            send_message.current_time = get_current_time();
            // espnow_send_data(macAddress[0][], (uint8_t *) &send_message, sizeof(send_message));
            LOGI(TAG, "RECEIVE VALVE ON RESPONSE CHIDL-%d", flowOrder[flowDoneCnt]);
            set_control_status(CHECK_FLOW);
          }
        } else if (recv_message.receive_type == SET_VALVE_OFF) {
          if (recv_message.resp == SUCCESS) {
            // 관수 완료를 HID 에 전달
            irrigation_message_t send_message;
            memset(&send_message, 0x00, sizeof(send_message));
            send_message.sender_type = ZONE_COMPLETE;
            send_message.deviceId = flowOrder[flowDoneCnt];  //  valve 를 off 할 child number
            send_message.current_time = get_current_time();
            // espnow_send_data(macAddress[0][], (uint8_t *) &send_message, sizeof(send_message));

            LOGI(TAG, "RECEIVE VALVE OFF RESPONSE CHIDL-%d", flowOrder[flowDoneCnt]);
            flowDoneCnt++;
            set_control_status(CHECK_SCEHDULE);
          }
        }
        break;

      default: break;
    }
  }
}

void on_data_sent_cb(const uint8_t* macAddr, esp_now_send_status_t status) {
  LOGI(TAG, "Delivery status : %d ", status);
}

void set_addr(void) {
  // syscfg 에서 읽어온 각 mac addr 을 저장...요거 구현 필요~~~~
  // 각 addr 을 어떤 형태로 저장해야 할지 논의 필요~~~₩
  int i, j, col_len, row_len;

  col_len = 6;
  row_len = 7;

  for (i = 0; i < row_len; i++) {
    for (j = 0; j < col_len; j++) {
      //  macAddress[i][j];  // mac addr 가져와서 넣어주는 부분 필요...
    }
  }
}

// if return 0 -> wake up 시간대
// return value -> sleep 시간까지 남은 시간, 단위 seconds
int check_sleep_time(void) {
  time_t currTime = get_current_time();
  struct tm timeinfo = { 0 };
  localtime_r(&currTime, &timeinfo);

  // 시간 sleep time / wake time 비교...
  // 오전 5시 ~ 9시 wake up...
  // 17시 ~ 21시 wake up...
  // 버퍼를 두기 위해 wake up 시간 대비 30분 전까지만 sleep
  if (timeinfo.tm_hour < 5) {
    if (timeinfo.tm_hour == 4 && timeinfo.tm_min >= 30) {
      return 0;
    }
    return ((4 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60);
  } else if (timeinfo.tm_hour >= 9 && timeinfo.tm_hour < 17) {
    if (timeinfo.tm_hour == 16 && timeinfo.tm_min >= 30) {
      return 0;
    }
    return ((16 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60);
  } else if (timeinfo.tm_hour >= 21) {
    return ((28 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60);
    ;
  } else {
    return 0;
  }
}

static void control_task(void* pvParameters) {
  irrigation_message_t send_message;
  for (;;) {
    switch (controlStatus) {
      case CHECK_ADDR:
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        // 각 device mac addr 존재 여부 판단... 확인 완료 시 check mode 단계..
        if (espnow_add_peers(MASTER_DEVICE)) {
          LOGI(TAG, "Success to add hid, child addr to peer list");
        } else {
          LOGI(TAG, "Failed to add hid, child addr to peer list");
        }
        LOG_BUFFER_HEXDUMP(TAG, macAddress, sizeof(macAddress), LOG_INFO);

        set_control_status(SYNC_TIME);

        LOGI(TAG, "ADDR CHECK DONE !!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        break;

#if 0
      case REGISTER_CB:
        // send / receive message callback 등록
        esp_now_register_recv_cb(on_data_recv);
        esp_now_register_send_cb(on_data_sent_cb);

        LOGI(TAG, "REGISTER CALLBACK DONE !!");
        set_control_status(SYNC_TIME);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        break;
#endif

      case SYNC_TIME:
        // master 시간을 기준으로 message 생성하여 broadcasting
        // 각 device 에서는 전달받은 시간 값으로 time sync
        uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        memset(&send_message, 0x00, sizeof(send_message));
        send_message.sender_type = TIME_SYNC;
        send_message.current_time = get_current_time();
        // espnow_send_data(broadcastAddress, (uint8_t *) &send_message, sizeof(send_message));

        LOGI(TAG, "HID/CHILD TIME SYNC !!");
        set_control_status(CHECK_SCEHDULE);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
        break;

      case WAIT_STATE:
        // 시간 체크 sleep mode 진입 여부 확인...
        // ESP Now 를 통해 message 받을 때 까지 대기 상태
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;

      case CHECK_SCEHDULE:
        // 현재 시간과 관수 스케쥴 시간을 비교 하는 단계.
        // 스케쥴 시간에 도달하면 valve on 과 함께 관수 시작

        if (setConfig) {  // 설정된 config가 있을 경우에 config 에 따라 스케줄 관리 시작
          if (!flowStart) {
            double diffTime = difftime(flowStartTime, get_current_time());

            if (diffTime < 2.0 && diffTime > (-10.0)) {
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
          int remainSleepTime = check_sleep_time();
          if (remainSleepTime > 0) {
            uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
            memset(&send_message, 0x00, sizeof(send_message));
            send_message.sender_type = SET_SLEEP;
            send_message.current_time = get_current_time();
            send_message.remain_time_sleep = remainSleepTime;
            // espnow_send_data(broadcastAddress, (uint8_t *) &send_message, sizeof(send_message));
            LOGI(TAG, "SEND DEEP SLEEP MSG ");
            set_control_status(WAIT_STATE);
          }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;

      case CHECK_FLOW:
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
        if (get_flow_value() >= (flowSettingValue - 20)) {
          set_control_status(CHILD_VALVE_OFF);
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
        break;

      case CHILD_VALVE_ON:
        // 관수 스케줄에 따라 pump / zone pump on, off 컨트롤
        // 관수 시작 시 : zone valve on -> pump on
        memset(&send_message, 0x00, sizeof(send_message));
        send_message.sender_type = SET_VALVE_ON;
        send_message.deviceId = flowOrder[flowDoneCnt];  //  valve 를 on 할 child number
        send_message.current_time = get_current_time();
        // espnow_send_data(macAddress[flowOrder[flowDoneCnt]][], (uint8_t *) &send_message, sizeof(send_message));
        LOGI(TAG, "CHILD -%d VALVE ON !!", flowOrder[flowDoneCnt]);
        set_control_status(WAIT_STATE);

        break;

      case CHILD_VALVE_OFF:
        // 관수 스케줄에 따라 pump / zone pump on, off 컨트롤
        // 관수 중지 시 : pump off -> zone valve on
        stop_flow();

        memset(&send_message, 0x00, sizeof(send_message));
        send_message.sender_type = SET_VALVE_OFF;
        send_message.deviceId = flowOrder[flowDoneCnt];  //  valve 를 off 할 child number
        send_message.current_time = get_current_time();
        // espnow_send_data(macAddress[flowOrder[flowDoneCnt]][], (uint8_t *) &send_message, sizeof(send_message));
        LOGI(TAG, "CHILD - %d VALVE OFF !!", flowOrder[flowDoneCnt]);
        set_control_status(WAIT_STATE);

        break;

      case COMPLETE:
        // 관수 스케줄대로 모든 관수가 종료된 상태.
        // 추가 관수 여부 확인..--> wait config 모드로
        memset(&send_message, 0x00, sizeof(send_message));
        send_message.sender_type = ALL_COMPLETE;
        send_message.current_time = get_current_time();
        // espnow_send_data(macAddress[0][], (uint8_t *) &send_message, sizeof(send_message));

        LOGI(TAG, "ALL CHILD FLOW COMPLETE!!");

        init_variable();
        set_control_status(CHECK_SCEHDULE);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;

      default: break;
    }
  }
}

void create_control_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (control_handle) {
    LOGI(TAG, "control task is alreay created");
    return;
  }

  init_variable();
  xTaskCreate(control_task, "CONTROL", stack_size, NULL, task_priority, &control_handle);
}
