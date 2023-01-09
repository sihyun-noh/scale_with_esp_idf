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
#include "main.h"
#include "adc.h"
#include "comm_packet.h"

static const char* TAG = "control_task";
static TaskHandle_t control_handle = NULL;

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
  ERROR
} condtrol_status_t;

condtrol_status_t controlStatus = CHECK_ADDR;

bool setConfig;        // config setting 여부
bool flowStart;        // 관수 시작 flag
time_t flowStartTime;  // 관수 시작 설정 시간
int zoneFlowCnt;       // 관수 설정된 zone 갯수
int flowOrder[6];      // 관수 순서
int flowSettingValue;  // 관수 설정 값
int zoneBattery[7];    // 0: master, 1~6: child 배터리 잔량

int flowDoneCnt;  // zone 변 관수 완료 카운트
int batteryCnt;   // child 배터리 수신 횟수

uint64_t remainSleepTime;  // sleep 시간

extern uint8_t macAddress[7][6];  // 0 = HID, 1~6 = child 1~6

void init_variable(void) {
  setConfig = false;
  flowStart = false;
  memset(&flowStartTime, 0x00, sizeof(flowStartTime));
  zoneFlowCnt = 0;
  memset(&flowOrder, 0x00, sizeof(flowOrder));
  memset(&zoneBattery, 0x00, sizeof(zoneBattery));
  flowSettingValue = 0;
  flowDoneCnt = 0;
  batteryCnt = 0;
  remainSleepTime = 0;
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

void get_addr(uint8_t arr[], int zone) {
  if (zone > 6) {
    memset(arr, 0xFF, sizeof(uint8_t) * 6);
  } else {
    for (int i = 0; i < 6; i++) {
      arr[i] = macAddress[zone][i];
    }
  }
}

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
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
  if (timeinfo.tm_hour < 5) {
    if (timeinfo.tm_hour == 4 && timeinfo.tm_min >= 30) {
      return 0;
    }
    return (uint64_t)(((4 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60));
  } else if (timeinfo.tm_hour >= 9 && timeinfo.tm_hour < 17) {
    if (timeinfo.tm_hour == 16 && timeinfo.tm_min >= 30) {
      return 0;
    }
    return (uint64_t)(((16 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60));
  } else if (timeinfo.tm_hour >= 21) {
    return (uint64_t)(((28 - timeinfo.tm_hour) * 3600) + ((30 - timeinfo.tm_min) * 60));
  } else {
    return 0;
  }
}

bool send_esp_data(message_type_t sender, int receiver) {
  irrigation_message_t send_message;
  memset(&send_message, 0x00, sizeof(send_message));
  send_message.sender_type = sender;
  send_message.current_time = get_current_time();
  uint8_t zoneAddr[6] = {
    0,
  };
  get_addr(zoneAddr, receiver);

  LOG_BUFFER_HEXDUMP(TAG, zoneAddr, sizeof(zoneAddr), LOG_INFO);

  switch (sender) {
    case START_FLOW: {
      send_message.deviceId = flowOrder[flowDoneCnt];
    } break;

    case ZONE_COMPLETE: {
      send_message.deviceId = flowOrder[flowDoneCnt];
      send_message.flow_value = get_flow_value();
    } break;

    case BATTERY_LEVEL: {
      memcpy(&(send_message.battery_level), zoneBattery, sizeof(send_message.battery_level));
    } break;

    case SET_SLEEP: {
      send_message.remain_time_sleep = remainSleepTime;
    } break;

    default: break;
  }

  return espnow_send_data(zoneAddr, (uint8_t*)&send_message, sizeof(send_message));
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
      case SET_CONFIG: {
        flowStartTime = recv_message.config.start_time;
        zoneFlowCnt = recv_message.config.zone_cnt;
        memcpy(flowOrder, recv_message.config.zones, sizeof(flowOrder));
        flowSettingValue = recv_message.config.flow_rate;
        flowDoneCnt = 0;
        setConfig = true;
        set_control_status(CHECK_SCEHDULE);
        LOGI(TAG, "RECEIVE & SET CONFIG from HID");
      } break;

      case RESPONSE: {
        if (recv_message.receive_type == SET_VALVE_ON) {
          if (recv_message.resp == SUCCESS) {
            start_flow();
            // 관수 시작을 HID 에 전달
            if (!send_esp_data(START_FLOW, 0))
              send_esp_data(START_FLOW, 0);

            LOGI(TAG, "RECEIVE VALVE ON RESPONSE CHILD-%d", flowOrder[flowDoneCnt]);
            set_control_status(CHECK_FLOW);
          }
        } else if (recv_message.receive_type == SET_VALVE_OFF) {
          if (recv_message.resp == SUCCESS) {
            // 관수 완료를 HID 에 전달
            if (!send_esp_data(ZONE_COMPLETE, 0))
              send_esp_data(ZONE_COMPLETE, 0);

            LOGI(TAG, "RECEIVE VALVE OFF RESPONSE CHILD-%d", flowOrder[flowDoneCnt]);
            flowDoneCnt++;
            reset_water_flow_liters();
            set_control_status(CHECK_SCEHDULE);
          }
        } else if (recv_message.receive_type == TIME_SYNC) {
          zoneBattery[recv_message.deviceId] = recv_message.battery_level[recv_message.deviceId];
          batteryCnt++;

          if (batteryCnt >= 1) {
            zoneBattery[0] = read_battery_percentage();
            LOGI(TAG, "Battery Level, Master: %d, Child : %d, %d, %d, %d, %d, %d ", zoneBattery[0], zoneBattery[1],
                 zoneBattery[2], zoneBattery[3], zoneBattery[4], zoneBattery[5], zoneBattery[6]);

            if (!send_esp_data(BATTERY_LEVEL, 0))
              send_esp_data(BATTERY_LEVEL, 0);

            batteryCnt = 0;
            set_control_status(CHECK_SCEHDULE);
          }
        }
      } break;

      default: break;
    }
  }
}

void on_data_sent_cb(const uint8_t* macAddr, esp_now_send_status_t status) {
  LOGI(TAG, "Delivery status : %d ", status);
}

static void control_task(void* pvParameters) {
  for (;;) {
    switch (controlStatus) {
      case CHECK_ADDR: {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        // 각 device mac addr 존재 여부 판단... 확인 완료 시 check mode 단계..
        if (espnow_add_peers(MASTER_DEVICE)) {
          LOGI(TAG, "Success to add hid, child addr to peer list");
        } else {
          LOGI(TAG, "Failed to add hid, child addr to peer list");
        }
        LOG_BUFFER_HEXDUMP(TAG, macAddress, sizeof(macAddress), LOG_INFO);

        set_control_status(CHECK_TIME);

        LOGI(TAG, "ADDR CHECK DONE !!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      } break;

      case CHECK_TIME: {
        time_t currTime = get_current_time();
        struct tm timeinfo = { 0 };
        localtime_r(&currTime, &timeinfo);

        if (timeinfo.tm_year < 120) {
          LOGI(TAG, "Time set not yet ");
        } else {
          set_control_status(SYNC_TIME);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
      } break;

      case SYNC_TIME: {
        // master 시간을 기준으로 message 생성하여 broadcasting
        // 각 device 에서는 전달받은 시간 값으로 time sync
        // 다른 device 별 rtc 차이로 깨어 나는 시간 차이를 위해 시간 버퍼 추가.
        // 10분으로 시간 버퍼 적용 --> 테스트 진행 후 값 튜닝 필요
        vTaskDelay((1000 * 60 * 10) / portTICK_PERIOD_MS);

        if (!send_esp_data(TIME_SYNC, 7))
          send_esp_data(TIME_SYNC, 7);

        LOGI(TAG, "SEND HID/CHILD TIME SYNC !!");
        set_control_status(WAIT_STATE);
      } break;

      case WAIT_STATE: {
        // 대기 상태
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      } break;

      case CHECK_SCEHDULE: {
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
          remainSleepTime = check_sleep_time();
          if (remainSleepTime > 0) {
            if (!send_esp_data(SET_SLEEP, 7))
              send_esp_data(SET_SLEEP, 7);

            LOGI(TAG, "SEND DEEP SLEEP MSG, SleepTime : %llus ", remainSleepTime);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            sleep_timer_wakeup(remainSleepTime);
          } else {
            LOGI(TAG, "WAIT SET CONFIG ");
          }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
        if (get_flow_value() >= (flowSettingValue - 20)) {
          set_control_status(CHILD_VALVE_OFF);
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
      } break;

      case CHILD_VALVE_ON: {
        // 관수 스케줄에 따라 pump / zone pump on, off 컨트롤
        // 관수 시작 시 : zone valve on -> pump on
        if (!send_esp_data(SET_VALVE_ON, flowOrder[flowDoneCnt]))
          send_esp_data(SET_VALVE_ON, flowOrder[flowDoneCnt]);

        LOGI(TAG, "CHILD -%d VALVE ON !!", flowOrder[flowDoneCnt]);
        set_control_status(WAIT_STATE);

      } break;

      case CHILD_VALVE_OFF: {
        // 관수 스케줄에 따라 pump / zone pump on, off 컨트롤
        // 관수 중지 시 : pump off -> zone valve off
        stop_flow();

        if (!send_esp_data(SET_VALVE_OFF, flowOrder[flowDoneCnt]))
          send_esp_data(SET_VALVE_OFF, flowOrder[flowDoneCnt]);

        LOGI(TAG, "CHILD - %d VALVE OFF !!", flowOrder[flowDoneCnt]);
        set_control_status(WAIT_STATE);

      } break;

      case COMPLETE: {
        // 관수 스케줄대로 모든 관수가 종료된 상태.
        // 추가 관수 여부 확인..--> wait config 모드로
        if (!send_esp_data(ALL_COMPLETE, 0))
          send_esp_data(ALL_COMPLETE, 0);

        LOGI(TAG, "ALL CHILD FLOW COMPLETE!!");

        init_variable();
        set_control_status(CHECK_SCEHDULE);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
      } break;

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
