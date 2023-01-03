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
#include "time_api.h"

static const char* TAG = "control_task";
static TaskHandle_t control_handle = NULL;

typedef enum {
  SET_CONFIG = 0,
  TIME_SYNC,
  START_FLOW,
  BATTERY_LEVEL,
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
  time_t start_time;
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
  int deviceId;
  int remain_time_sleep;
  int battery_level[7];  // 0: HID, 1~6: child 1~6
  time_t current_time;
} irrigation_message_t;

typedef enum { CHECK_ADDR = 0, WAIT_STATE, ERROR } condtrol_status_t;

condtrol_status_t controlStatus = CHECK_ADDR;

extern uint8_t masterAddress[6];

int myId = 1;

void init_variable(void) {
  myId = -1;
  memset(&masterAddress, 0x00, sizeof(masterAddress));
}

void set_control_status(condtrol_status_t value) {
  controlStatus = value;
}

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

bool send_esp_data(message_type_t sender, message_type_t receiver) {
  irrigation_message_t send_message;
  memset(&send_message, 0x00, sizeof(send_message));
  send_message.sender_type = sender;
  send_message.receive_type = receiver;
  send_message.resp = SUCCESS;
  send_message.deviceId = myId; 
  send_message.current_time = get_current_time();

  if (receiver == TIME_SYNC) {
    send_message.battery_level[myId] = read_battery_percentage();
  }

  return espnow_send_data(masterAddress, (uint8_t *) &send_message, sizeof(send_message)) == ESP_OK;
}

void on_data_recv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  irrigation_message_t recv_message;
  memcpy(&recv_message, incomingData, sizeof(recv_message));

  LOGI(TAG, "Receive Data from Master");
  LOG_BUFFER_HEXDUMP(TAG, incomingData, len, LOG_INFO);

  if (len > 0) {
    // CHILD 에서는 master cmd 처리만
    // SYNC_TIME, Valve on/off, sleep mode
    // Child 의 배터리 잔량 값. --> 좀더 확인 필요
    switch (recv_message.sender_type) {
      case TIME_SYNC: {
        time_t syncTimeValue = recv_message.current_time;
        // time sync 동작...
        struct tm timeinfo = { 0 };
        localtime_r(&syncTimeValue, &timeinfo);

        set_local_time(timeinfo.tm_year+1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        LOGI(TAG, "Time synced zone : %d !!", myId);

        // 완료 후 내 device ID 와 battery 값을 master 에 전송..
        if (!send_esp_data(RESPONSE, TIME_SYNC))
          send_esp_data(RESPONSE, TIME_SYNC);

        LOGI(TAG, "Zone-%d Battery level send", myId);
      } break;

      case SET_VALVE_ON: {
        // valve on -> send response message
        valve_open();
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (!send_esp_data(RESPONSE, SET_VALVE_ON))
          send_esp_data(RESPONSE, SET_VALVE_ON);

        LOGI(TAG, "Zone-%d Valve On", myId);
      } break;

      case SET_VALVE_OFF: {
        // valve off -> send response message
        valve_close();
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (!send_esp_data(RESPONSE, SET_VALVE_OFF))
          send_esp_data(RESPONSE, SET_VALVE_OFF);

        LOGI(TAG, "Zone-%d Valve Off", myId);
      } break;

      case SET_SLEEP: {
        LOGI(TAG, "Start sleep");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sleep_timer_wakeup(recv_message.remain_time_sleep);
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
      case CHECK_ADDR:
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        // 각 master Mac Addr 얻어와서 변수에 저장하는 단계..
        if (espnow_add_peers(CHILD_DEVICE)) {
          LOGI(TAG, "Success to add master addr to peer list");
        } else {
          LOGI(TAG, "Failed to add master addr to peer list");
        }
        LOG_BUFFER_HEXDUMP(TAG, masterAddress, sizeof(masterAddress), LOG_INFO);

        myId = get_address_matching_id();

        if (myId == (-1)) {
          LOGI(TAG, "Fail get child ID !!");
        } else {
          LOGI(TAG, "matching get id : %d", myId);
        }

        set_control_status(WAIT_STATE);       

        LOGI(TAG, "ADDR CHECK DONE !!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        break;

      case WAIT_STATE:
        // ESP Now 를 통해 message 받을 때 까지 대기 상태
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
