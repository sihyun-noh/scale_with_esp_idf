
// sensor_auto_detect.h

#ifndef SENSOR_AUTO_DETECT_H
#define SENSOR_AUTO_DETECT_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SENSOR_PORTS 6

// Macro to get event bit per port
#define SENSOR_PORT_EVENT_BIT(portId) (1 << (portId))

extern SemaphoreHandle_t sensor_mutex;
// extern EventGroupHandle_t sensor_event_group;

typedef struct {
  int data;
  EventGroupHandle_t sensor_event_group;  ///< Shared event group
  SemaphoreHandle_t data_mutex;
} sensor_ad_manager_t;

/**
 * @brief Structure to hold SDI-12 sensor information.
 */
typedef struct {
  char addr[4];                 ///< SDI-12 sensor address
  char version[8];              ///< SDI-12 version string
  char type[16];                ///< Sensor type or manufacturer
  uint32_t lastCheckTimestamp;  ///< Last checked timestamp (ms)
} sensor_info_t;

/**
 * @brief Context structure for each sensor port.
 */
typedef struct {
  int portId;                   ///< Unique port identifier
  int detectPin;                ///< GPIO pin for detecting plug/unplug
  int controlPin;               ///< GPIO pin to control sensor activation
  int oePin;                    ///< Optional OE (Output Enable) pin
  bool isSensorConnected;       ///< Current connection status
  bool hasSensorChanged;        ///< Whether sensor has changed
  sensor_info_t currentSensor;  ///< Current sensor info
  sensor_info_t lastSensor;     ///< Last known sensor info
} sensor_port_ctx_t;

/**
 * @brief Top-level structure to manage all sensor ports.
 */
typedef struct {
  sensor_port_ctx_t ports[MAX_SENSOR_PORTS];
  int portCount;
} sensor_system_t;

/**
 * @brief Initialize sensor detection system using Kconfig-defined pins.
 */
void sensor_auto_detect_init(void);

/**
 * @brief Sensor detection event handler task
 * @param arg Not used, pass NULL
 */
void sensor_auto_detect_task(void* arg);

/**
 * @brief Check if sensor is connected on port
 * @param portId Port number (0-based)
 * @return true if connected
 */
bool sensor_is_connected(int portId);

/**
 * @brief Control the sensor buffer MUX
 */
void sensor_buffer_control_init(void);
esp_err_t sensor_buffer_select_port(int portId);
esp_err_t sensor_buffer_disable(void);

// 싱글톤 인스턴스 가져오기
sensor_ad_manager_t* sensor_ad_get_instance(void);
bool sensor_ad_is_initialized(void);

// 안전한 접근 함수들
void my_manager_set_data(int val);
int my_manager_get_data(void);
void my_manager_add_data(int delta);

#ifdef __cplusplus
}
#endif

#endif  // SENSOR_AUTO_DETECT_H
