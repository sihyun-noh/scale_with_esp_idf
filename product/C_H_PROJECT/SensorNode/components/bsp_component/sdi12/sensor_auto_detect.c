

// sensor_auto_detect.c

#include <string.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "sdkconfig.h"

#include "sensor_auto_detect.h"

#define DEBOUNCE_TIME_MS 100  // 디바운스 임계 시간

static const char *TAG = "Sensor_AutoDetect";

static sensor_system_t sensorSys;
SemaphoreHandle_t sensor_mutex = NULL;
// EventGroupHandle_t sensor_event_group = NULL;

static sensor_ad_manager_t instance;
static SemaphoreHandle_t singleton_mutex = NULL;
static bool initialized = false;

static QueueHandle_t sensor_event_queue;

typedef struct {
  int portId;
} sensor_event_t;

// Dummy sensor query function (simulate sensor response)
static sensor_info_t sdi12_query_sensor(int portId) {
  sensor_info_t info = { 0 };
  if (portId >= 0 && portId < MAX_SENSOR_PORTS) {
    sprintf(info.addr, "%d", portId);
    strcpy(info.version, "1.3");
    strcpy(info.type, "Apogee");
    info.lastCheckTimestamp = esp_timer_get_time() / 1000;
  }
  return info;
}

sensor_ad_manager_t *sensor_ad_get_instance(void) {
  // 초기화 보호
  if (singleton_mutex == NULL) {
    singleton_mutex = xSemaphoreCreateMutex();
    if (singleton_mutex == NULL) {
      ESP_LOGE(TAG, "Failed to create mutex");
      return NULL;
    }
  }

  if (xSemaphoreTake(singleton_mutex, portMAX_DELAY)) {
    if (!initialized) {
      // 기본 필드 초기화
      instance.data = 0;

      // 이벤트 그룹 생성
      instance.sensor_event_group = xEventGroupCreate();
      if (instance.sensor_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        xSemaphoreGive(singleton_mutex);
        return NULL;
      }

      // 싱글톤 시 뮤택스 초기화
      instance.data_mutex = xSemaphoreCreateMutex();
      if (instance.data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create data mutex");
      }

      initialized = true;
      ESP_LOGI(TAG, "initialized");
    }
    xSemaphoreGive(singleton_mutex);
  } else {
    ESP_LOGE(TAG, "Mutex lock failed");
    return NULL;
  }

  return &instance;
}

bool sensor_ad_is_initialized(void) {
  return initialized;
}

void my_manager_set_data(int val) {
  sensor_ad_manager_t *instance = sensor_ad_get_instance();
  if (instance && instance->data_mutex) {
    xSemaphoreTake(instance->data_mutex, portMAX_DELAY);
    instance->data = val;
    xSemaphoreGive(instance->data_mutex);
  }
}

int my_manager_get_data(void) {
  sensor_ad_manager_t *instance = sensor_ad_get_instance();
  int result = 0;
  if (instance && instance->data_mutex) {
    xSemaphoreTake(instance->data_mutex, portMAX_DELAY);
    result = instance->data;
    xSemaphoreGive(instance->data_mutex);
  }
  return result;
}

void my_manager_add_data(int delta) {
  sensor_ad_manager_t *instance = sensor_ad_get_instance();
  if (instance && instance->data_mutex) {
    xSemaphoreTake(instance->data_mutex, portMAX_DELAY);
    instance->data += delta;
    xSemaphoreGive(instance->data_mutex);
  }
}

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  int portId = (int)arg;
  sensor_event_t evt = { .portId = portId };
  xQueueSendFromISR(sensor_event_queue, &evt, NULL);
}

void sensor_buffer_control_init(void) {
  gpio_config_t io_conf = {
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };

  int buffer_pins[] = {
    CONFIG_SENSOR_BUFFER_ENABLE_PIN,
    CONFIG_SENSOR_BUFFER_S0_PIN,
    CONFIG_SENSOR_BUFFER_S1_PIN,
    CONFIG_SENSOR_BUFFER_S2_PIN,
  };

  for (int i = 0; i < 4; ++i) {
    io_conf.pin_bit_mask = (1ULL << buffer_pins[i]);
    gpio_config(&io_conf);
    gpio_set_level(buffer_pins[i], 0);
  }

  ESP_LOGI(TAG, "Sensor buffer control initialized.");
}

esp_err_t sensor_buffer_select_port(int portId) {
  if (gpio_set_level(CONFIG_SENSOR_BUFFER_ENABLE_PIN, 0) != ESP_OK)
    return ESP_FAIL;

  if (gpio_set_level(CONFIG_SENSOR_BUFFER_S0_PIN, (portId >> 0) & 0x01) != ESP_OK)
    return ESP_FAIL;
  if (gpio_set_level(CONFIG_SENSOR_BUFFER_S1_PIN, (portId >> 1) & 0x01) != ESP_OK)
    return ESP_FAIL;
  if (gpio_set_level(CONFIG_SENSOR_BUFFER_S2_PIN, (portId >> 2) & 0x01) != ESP_OK)
    return ESP_FAIL;
  ESP_LOGI(TAG, "Buffer set to port %d (MUX: %d%d%d)", portId, (portId >> 2) & 0x01, (portId >> 1) & 0x01,
           (portId >> 0) & 0x01);
  return ESP_OK;
}

esp_err_t sensor_buffer_disable(void) {
  esp_err_t ret;
  ret = gpio_set_level(CONFIG_SENSOR_BUFFER_ENABLE_PIN, 1);
  ESP_LOGI(TAG, "Buffer disabled.");
  return ret;
}

void sensor_auto_detect_init(void) {
  // sensor_event_group = xEventGroupCreate();
  sensor_event_queue = xQueueCreate(10, sizeof(sensor_event_t));
  sensor_mutex = xSemaphoreCreateMutex();
  sensorSys.portCount = MAX_SENSOR_PORTS;

  sensor_buffer_control_init();

  const int detect_pins[MAX_SENSOR_PORTS] = {
    CONFIG_SENSOR_PORT_0_DETECT_PIN, CONFIG_SENSOR_PORT_1_DETECT_PIN, CONFIG_SENSOR_PORT_2_DETECT_PIN,
    CONFIG_SENSOR_PORT_3_DETECT_PIN, CONFIG_SENSOR_PORT_4_DETECT_PIN, CONFIG_SENSOR_PORT_5_DETECT_PIN,
  };

  const int control_pins[MAX_SENSOR_PORTS] = {
    CONFIG_SENSOR_PORT_0_CTRL_PIN, CONFIG_SENSOR_PORT_1_CTRL_PIN, CONFIG_SENSOR_PORT_2_CTRL_PIN,
    CONFIG_SENSOR_PORT_3_CTRL_PIN, CONFIG_SENSOR_PORT_4_CTRL_PIN, CONFIG_SENSOR_PORT_5_CTRL_PIN,
  };

  for (int i = 0; i < MAX_SENSOR_PORTS; i++) {
    sensorSys.ports[i].portId = i;
    sensorSys.ports[i].detectPin = detect_pins[i];
    sensorSys.ports[i].controlPin = control_pins[i];
    sensorSys.ports[i].oePin = -1;

    gpio_config_t io_conf = { .pin_bit_mask = (1ULL << sensorSys.ports[i].detectPin),
                              .mode = GPIO_MODE_INPUT,
                              .pull_up_en = GPIO_PULLUP_ENABLE,
                              .intr_type = GPIO_INTR_ANYEDGE };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(sensorSys.ports[i].detectPin, gpio_isr_handler, (void *)i);

    gpio_reset_pin(sensorSys.ports[i].controlPin);
    gpio_set_direction(sensorSys.ports[i].controlPin, GPIO_MODE_OUTPUT);
    gpio_set_level(sensorSys.ports[i].controlPin, 0);
  }

  ESP_LOGI(TAG, "Sensor detection system initialized.");
}
#if 0
void sensor_auto_detect_task(void *arg) {
  sensor_event_t evt;

  while (1) {
    if (xQueueReceive(sensor_event_queue, &evt, portMAX_DELAY)) {
      if (xSemaphoreTake(sensor_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        sensor_port_ctx_t *port = &sensorSys.ports[evt.portId];

        gpio_set_level(port->controlPin, 1);
        vTaskDelay(pdMS_TO_TICKS(500));

        bool wasConnected = port->isSensorConnected;

        // TODO: read to sensor info

        sensor_info_t newSensor = sdi12_query_sensor(port->portId);

        if (strlen(newSensor.addr) > 0) {
          port->isSensorConnected = true;
          if (!wasConnected) {
            ESP_LOGI(TAG, "[PORT %d] Sensor connected (addr: %s)", port->portId, newSensor.addr);
          }

          port->hasSensorChanged = memcmp(&newSensor, &port->currentSensor, sizeof(sensor_info_t)) != 0;
          if (port->hasSensorChanged) {
            port->lastSensor = port->currentSensor;
            port->currentSensor = newSensor;
            ESP_LOGI(TAG, "[PORT %d] Sensor updated (type: %s)", port->portId, newSensor.type);
          }
          // Set event bit for connected sensor
          xEventGroupSetBits(sensor_event_group, SENSOR_PORT_EVENT_BIT(port->portId));
        } else {
          port->isSensorConnected = false;
          if (wasConnected) {
            ESP_LOGW(TAG, "[PORT %d] Sensor disconnected", port->portId);
            memset(&port->currentSensor, 0, sizeof(sensor_info_t));
            // Clear event bit for disconnected sensor
            xEventGroupClearBits(sensor_event_group, SENSOR_PORT_EVENT_BIT(port->portId));
          }
        }

        gpio_set_level(port->controlPin, 0);
        xSemaphoreGive(sensor_mutex);
      }
    }
  }
}
#endif

void sensor_auto_detect_task(void *arg) {
  sensor_ad_manager_t *mgr = sensor_ad_get_instance();
  sensor_event_t evt;
  static int64_t last_event_time[MAX_SENSOR_PORTS] = { 0 };
  static int last_pin_level[MAX_SENSOR_PORTS] = { 0 };  // 이전 상태 저장용

  while (1) {
    //// 여기부터 기존 코드
    if (xQueueReceive(sensor_event_queue, &evt, portMAX_DELAY)) {
      int portId = evt.portId;
      int64_t now = esp_timer_get_time() / 1000;  // in ms

      // 디바운스: 마지막 이벤트 이후 일정 시간 이내면 무시
      if (now - last_event_time[portId] < DEBOUNCE_TIME_MS) {
        continue;
      }
      last_event_time[portId] = now;

      int current_level = gpio_get_level(sensorSys.ports[portId].detectPin);
      int previous_level = last_pin_level[portId];
      last_pin_level[portId] = current_level;

      if (previous_level == 0 && current_level == 1) {
        ESP_LOGI(TAG, "[PORT %d] RISING EDGE detected", portId);
        gpio_set_level(sensorSys.ports[portId].controlPin, 1);
        vTaskDelay(pdMS_TO_TICKS(250));
        // 센서 연결 처리 (3.3V 입력)
      } else if (previous_level == 1 && current_level == 0) {
        ESP_LOGI(TAG, "[PORT %d] FALLING EDGE detected", portId);
        gpio_set_level(sensorSys.ports[portId].controlPin, 0);
        vTaskDelay(pdMS_TO_TICKS(250));
        // 센서 분리 처리 (0V 입력)
      } else {
        continue;
      }

      if (xSemaphoreTake(sensor_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        sensor_port_ctx_t *port = &sensorSys.ports[portId];
        bool wasConnected = port->isSensorConnected;

        // 현재 GPIO 상태 확인
        // int level = gpio_get_level(port->detectPin);

        if (current_level == 1) {
          // rising edge 감지: 센서 연결됨
          port->isSensorConnected = true;

          sensor_buffer_select_port(portId);
          sensor_info_t newSensor = sdi12_query_sensor(port->portId);
          sensor_buffer_disable();

          if (!wasConnected) {
            ESP_LOGI(TAG, "[PORT %d] Sensor connected (addr: %s)", portId, newSensor.addr);
          }

          port->hasSensorChanged = memcmp(&newSensor, &port->currentSensor, sizeof(sensor_info_t)) != 0;
          if (port->hasSensorChanged) {
            port->lastSensor = port->currentSensor;
            port->currentSensor = newSensor;
            ESP_LOGI(TAG, "[PORT %d] Sensor updated (type: %s)", portId, newSensor.type);
          }

          xEventGroupSetBits(mgr->sensor_event_group, SENSOR_PORT_EVENT_BIT(portId));
        } else {
          // falling edge 감지: 센서 연결 해제됨
          port->isSensorConnected = false;

          if (wasConnected) {
            ESP_LOGW(TAG, "[PORT %d] Sensor disconnected", portId);
            memset(&port->currentSensor, 0, sizeof(sensor_info_t));
            xEventGroupClearBits(mgr->sensor_event_group, SENSOR_PORT_EVENT_BIT(portId));
          }
        }

        xSemaphoreGive(sensor_mutex);
      }
    }
  }
}

bool sensor_is_connected(int portId) {
  if (portId < 0 || portId >= sensorSys.portCount)
    return false;
  return sensorSys.ports[portId].isSensorConnected;
}
