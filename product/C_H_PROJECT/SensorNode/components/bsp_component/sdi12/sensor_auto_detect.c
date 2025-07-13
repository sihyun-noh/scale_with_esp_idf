

// sensor_auto_detect.c

#include <string.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "sdi12_task.h"
#include "sdkconfig.h"

#include "sensor_auto_detect.h"
#include "sensor_cfg_config.h"
#include "bsp_gpio.h"

#define DEBOUNCE_TIME_MS 100
static const char *TAG = "[sensorAutoDetect]";

static sensor_system_t sensorSys;
SemaphoreHandle_t sensor_mutex = NULL;

static sensor_ad_manager_t instance;
static SemaphoreHandle_t singleton_mutex = NULL;
static bool initialized = false;

static QueueHandle_t sensor_event_queue;
static int isr_port_ids[MAX_SENSOR_PORTS];  // ISR에 넘길 고정된 포인터 저장

typedef struct {
  int portId;
} sensor_event_t;

/* clang-format off */
/**
 * @brief Sensor model mapping tables and structure.
 *
 * Defines a mapping between reported sensor model names,
 * internal sensor types, and expected column sizes.
 *
 * Includes separate tables for METER and APOGEE manufacturers,
 * enabling automatic sensor configuration when detected.
 */
typedef struct {
  const char *model;
  sdi12_sensor_type_t type;
  uint8_t columns_size;
} sensor_model_map_t;

// METER Model map
static const sensor_model_map_t meter_model_map[] = {
  { "TER11", TEROS11, 2 },
  { "TER12", TEROS12, 3 },
  { "TER21", TEROS21, 2 },
  { "TER54", TEROS54, 8 },
  { "ATMOS22", ATMOS22, 9 },
  { "ATMOS41", ATMOS41, 17 },
};

// APOGEE Model map
static const sensor_model_map_t apogee_model_map[] = {
  { "S2-412", APOGEE_S2_412, 1 },  // NDVI
  { "SQ-521", APOGEE_SQ_521, 1 },  // PPFD
};
/* clang-format on */

sensor_system_t *sensor_ctl_get_instance(void) {
  return &sensorSys;
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
  int *portId = (int *)arg;
  ESP_EARLY_LOGW(TAG, "ISR Triggered on port [%d]", *portId);
  sensor_event_t evt = { .portId = *portId };
  // xQueueSendFromISR(sensor_event_queue, &evt, NULL);
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  BaseType_t ok = xQueueSendFromISR(sensor_event_queue, &evt, &xHigherPriorityTaskWoken);
  if (ok != pdTRUE) {
    ESP_EARLY_LOGW(TAG, "Failed to send event to queue from ISR");
  }
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();  // 컨텍스트 스위칭
  }
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

// S0 S1 S2
// L  L  L  == 0ch
//
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
  sensor_event_queue = xQueueCreate(20, sizeof(sensor_event_t));
  configASSERT(sensor_event_queue != NULL);

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

    gpio_config_t io_conf = { .pin_bit_mask = (1ULL << sensorSys.ports[i].detectPin),
                              .mode = GPIO_MODE_INPUT,
                              //.pull_up_en = GPIO_PULLUP_ENABLE,
                              .pull_down_en = GPIO_PULLDOWN_DISABLE,
                              .intr_type = GPIO_INTR_ANYEDGE };
    //.intr_type = GPIO_INTR_POSEDGE };
    gpio_config(&io_conf);

    if (i == 0) {
      gpio_install_isr_service(0);  // 최초 한 번만 설치
    }

    isr_port_ids[i] = i;
    gpio_isr_handler_add(sensorSys.ports[i].detectPin, gpio_isr_handler, (void *)&isr_port_ids[i]);

    gpio_reset_pin(sensorSys.ports[i].controlPin);
    gpio_set_direction(sensorSys.ports[i].controlPin, GPIO_MODE_OUTPUT);
    gpio_set_level(sensorSys.ports[i].controlPin, 0);
  }

  ESP_LOGI(TAG, "Sensor detection system initialized.");
}

/**
 * @brief Set control pin level for a specific sensor port.
 */
esp_err_t sensor_control_pin_set(int portId, int level) {
  if (portId < 0 || portId >= MAX_SENSOR_PORTS) {
    ESP_LOGE(TAG, "Invalid portId: %d", portId);
    return ESP_ERR_INVALID_ARG;
  }

  gpio_num_t pin = sensorSys.ports[portId].controlPin;
  gpio_set_level(pin, level);
  ESP_LOGI(TAG, "Set control pin %d (port %d) to %s", pin, portId, level ? "HIGH" : "LOW");
  return ESP_OK;
}

/**
 * @brief Get current GPIO level of control pin.
 */
int sensor_control_pin_get_level(int portId) {
  if (portId < 0 || portId >= MAX_SENSOR_PORTS) {
    ESP_LOGE(TAG, "Invalid portId: %d", portId);
    return -1;
  }

  gpio_num_t pin = sensorSys.ports[portId].controlPin;
  int level = gpio_get_level(pin);
  ESP_LOGI(TAG, "Read control pin %d (port %d): %s", pin, portId, level ? "HIGH" : "LOW");
  return level;
}

static void update_sensor_port_config(sensor_port_cfg_t *cfg, int portId, sdi12_sensor_type_t type,
                                      uint8_t columns_size) {
  /**
   * @brief Update and lock sensor configuration for the given port.
   *
   * This section updates the sensor configuration table (cfg) for the specified portId.
   * The function ensures thread safety by locking the configuration before making changes,
   * and unlocking it afterward.
   *
   * Fields set:
   *  - port: Logical port number (1-based index)
   *  - sensor_type: Type of the connected SDI-12 sensor
   *  - current_state: Marks port as ready
   *  - columns_size: Number of data columns for this sensor type
   *  - rows_size: Fixed to 1 (single row of data)
   *  - publish_interval: Default interval for server publishing
   *  - dirty: Marks this configuration as modified for persistence
   */
  sensor_cfg_lock();
  cfg[portId].port = portId + 1;
  cfg[portId].sensor_type = type;
  cfg[portId].current_state = SENSOR_PORT_STATUS_READY;
  cfg[portId].columns_size = columns_size;
  cfg[portId].rows_size = 1;
  cfg[portId].server_config.publish_interval = 1;
  cfg[portId].dirty = 1;
  sensor_cfg_unlock();

  // Save to NVS
  sensor_port_cfg_commit();

  // Blink LED
  blink_status_set_leds(LED_GREEN);
  vTaskDelay(pdMS_TO_TICKS(1000));
  blink_status_clear_leds(LED_GREEN);

  // Force reboot
  esp_restart();
}

static bool find_model_mapping(const sensor_model_map_t *table, size_t table_size, const char *model,
                               sdi12_sensor_type_t *type_out, uint8_t *columns_out) {
  for (size_t i = 0; i < table_size; ++i) {
    if (strcmp(model, table[i].model) == 0) {
      *type_out = table[i].type;
      *columns_out = table[i].columns_size;
      return true;
    }
  }
  return false;
}

void sensor_auto_detect_task(void *arg) {
  sensor_ad_manager_t *mgr = sensor_ad_get_instance();
  sensor_event_t evt;
  static int64_t last_event_time[MAX_SENSOR_PORTS] = { 0 };
  static int last_pin_level[MAX_SENSOR_PORTS] = { 1 };  // 이전 상태 저장용

  while (1) {
    if (xQueueReceive(sensor_event_queue, &evt, portMAX_DELAY)) {
      // NOTE:
      // Delay to prevent transient ISR triggers, e.g., during power loss.
      vTaskDelay(pdMS_TO_TICKS(1000));

      int portId = evt.portId;
      ESP_LOGE(TAG, "[PORT %d] ISR detected", portId);

      int64_t now = esp_timer_get_time() / 1000;  // in ms

      // Debounce: ignore if within a certain time after the last event
      if (now - last_event_time[portId] < DEBOUNCE_TIME_MS) {
        vTaskDelay(pdMS_TO_TICKS(10));  // 추가
        continue;
      }
      last_event_time[portId] = now;

      int current_level = gpio_get_level(sensorSys.ports[portId].detectPin);
      int previous_level = last_pin_level[portId];
      last_pin_level[portId] = current_level;

      if (previous_level == 1 && current_level == 0) {
        ESP_LOGI(TAG, "[PORT %d] FALLING EDGE detected", portId);
      } else if (previous_level == 0 && current_level == 1) {
        ESP_LOGI(TAG, "[PORT %d] RISING EDGE detected", portId);
      }

      sensor_port_ctx_t *port = &sensorSys.ports[portId];

      // port config 에 센서 정보및 상태 저장 하기위해 인스턴스 호출
      sensor_port_cfg_t *cfg = sensor_cfg_instance();
      if (!cfg) {
        ESP_LOGE(TAG, "sensor_cfg_instance() returned NULL");
      }
      // 비교하기 위한 변수
      sensor_port_cfg_t *cmp_empty = &cfg[portId];

      if (current_level == 0) {
        // NOTE: Falling edge detection: indicates sensor is connected
        //  Stop all tasks currently executing in the strategy task
        ESP_LOGW(TAG, "[Auto_Detect] Enable connected port [%d] ", portId + 1);
        sensor_ad_manager_t *mgr = sensor_ad_get_instance();
        for (int i = 0; i < SENSOR_PORT_COUNT; ++i) {
          ESP_LOGW(TAG, "[Auto_Detect] clear bit [%d] ", i);
          xEventGroupClearBits(mgr->sensor_event_group, SENSOR_PORT_EVENT_BIT(i));
        }

        // MUX control
        sensor_buffer_select_port(portId);
        int info_call_count = 0;
        sdi12_sensor_info_t *new_sensor_info = sdi12_info_start(portId);
        vTaskDelay(pdMS_TO_TICKS(1000));

        while (1) {
          // TODO: State LED
          // Operation to retrieve SDI-12 sensor information
          blink_status_set_leds(LED_RED);
          vTaskDelay(pdMS_TO_TICKS(500));
          blink_status_clear_leds(LED_RED);

          // 공백제거
          trim(new_sensor_info->manufacturer);
          trim(new_sensor_info->model);
          ESP_LOGI(TAG, "manufacturer:%s", new_sensor_info->manufacturer);
          ESP_LOGI(TAG, "model:%s", new_sensor_info->model);

          char *str = new_sensor_info->manufacturer;
          ESP_LOGI(TAG, "Manufacturer (hex):");
          for (int i = 0; i < strlen(str); i++) {
            printf("%02X ", (unsigned char)str[i]);
          }
          printf("\n");

          if (strcmp(new_sensor_info->manufacturer, "METER") == 0) {
            ESP_LOGW(TAG, "Sensor manufacturer : %s", new_sensor_info->manufacturer);
            ESP_LOGW(TAG, "Sensor type : %s", new_sensor_info->model);
            break;
          } else if (strcmp(new_sensor_info->manufacturer, "APOGEE") == 0) {
            ESP_LOGW(TAG, "Sensor type : %s", new_sensor_info->manufacturer);
            break;
          } else {
            new_sensor_info = sdi12_info_start(portId);
            info_call_count++;
            if (info_call_count == 10) {
              blink_status_set_leds(LED_BLUE);
              vTaskDelay(pdMS_TO_TICKS(1500));
              blink_status_clear_leds(LED_BLUE);
              esp_restart();
            }
          }
        };

        // TODO: Check sensor information, save it to NVS, then reboot.
        // The saved configuration type is sensor_port_cfg.
        // Structure that works based on a table map
        if (strcmp(new_sensor_info->manufacturer, "METER") == 0 ||
            strcmp(new_sensor_info->manufacturer, "APOGEE") == 0) {
          const sensor_model_map_t *map_table = NULL;
          size_t map_size = 0;

          if (strcmp(new_sensor_info->manufacturer, "METER") == 0) {
            map_table = meter_model_map;
            map_size = sizeof(meter_model_map) / sizeof(sensor_model_map_t);
          } else if (strcmp(new_sensor_info->manufacturer, "APOGEE") == 0) {
            map_table = apogee_model_map;
            map_size = sizeof(apogee_model_map) / sizeof(sensor_model_map_t);
          }

          sdi12_sensor_type_t detected_type;
          uint8_t columns_size;

          if (find_model_mapping(map_table, map_size, new_sensor_info->model, &detected_type, &columns_size)) {
            if (cmp_empty->sensor_type != detected_type) {
              ESP_LOGW(TAG, "Target sensor type mismatch, current:%d, detected:%d", cmp_empty->sensor_type,
                       detected_type);
              update_sensor_port_config(cfg, portId, detected_type, columns_size);
            } else {
              ESP_LOGW(TAG, "Target sensor type matched:[%d]", cmp_empty->sensor_type);
              blink_status_set_leds(LED_BLUE);
              vTaskDelay(pdMS_TO_TICKS(1000));
              blink_status_clear_leds(LED_BLUE);
            }
          } else {
            ESP_LOGE(TAG, "Unknown model for %s: %s", new_sensor_info->manufacturer, new_sensor_info->model);
          }
        } else {
          ESP_LOGE(TAG, "Unsupported manufacturer: %s", new_sensor_info->manufacturer);
        }

        sensor_buffer_disable();

      } else {
        ESP_LOGW(TAG, "[Auto_Detect] Disable connected port [%d] ", portId + 1);
        sensor_cfg_lock();
        cfg[portId].port = portId + 1;
        cfg[portId].sensor_type = SENSOR_TYPE_UNKNOWN;
        cfg[portId].current_state = SENSOR_PORT_STATUS_NONE;
        cfg[portId].dirty = 1;  // NVS updated flag bit.
        sensor_cfg_unlock();

        // NVS Save
        ESP_LOGW(TAG, "[Auto_Detect] Disable connected port [%d] ", portId + 1);
        sensor_port_cfg_commit();
        ESP_LOGW(TAG, "[Auto_Detect] Set bit [%d] ", portId);
        xEventGroupSetBits(mgr->sensor_event_group, SENSOR_PORT_EVENT_BIT(portId));
      }
    }  // xQueueReceive

  }  // while end
  vTaskDelay(pdMS_TO_TICKS(10));
}

bool sensor_is_connected(int portId) {
  if (portId < 0 || portId >= sensorSys.portCount)
    return false;
  return sensorSys.ports[portId].isSensorConnected;
}
