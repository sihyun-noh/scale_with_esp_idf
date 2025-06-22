
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

/**
 * @brief Set control pin level for a specific sensor port.
 *
 * @param portId Sensor port ID (0 ~ MAX_SENSOR_PORTS-1)
 * @param level GPIO level (0 = LOW, 1 = HIGH)
 * @return esp_err_t
 */
esp_err_t sensor_control_pin_set(int portId, int level);

/**
 * @brief Get current GPIO level of control pin.
 *
 * @param portId Sensor port ID (0 ~ MAX_SENSOR_PORTS-1)
 * @return int GPIO level (0 or 1), -1 if error
 */
int sensor_control_pin_get_level(int portId);

/**
 * @brief Selects the specified sensor port using the MUX control pins.
 *
 * Sets S0, S1, S2 pins based on portId (0~7) to route the buffer to the correct sensor port.
 * Automatically disables the buffer (active-low) before switching.
 *
 * @param portId Target port to select (0 ~ 7)
 * @return ESP_OK on success, ESP_FAIL on GPIO error
 */
esp_err_t sensor_buffer_select_port(int portId);

/**
 * @brief Disables the sensor buffer.
 *
 * Sets the buffer enable pin high to disable the output.
 *
 * @return ESP_OK on success, ESP_FAIL on GPIO error
 */
esp_err_t sensor_buffer_disable(void);

/**
 * @brief Returns the singleton instance of the sensor auto-detect manager.
 *
 * Initializes the instance on first call.
 *
 * @return Pointer to sensor_ad_manager_t instance
 */
sensor_ad_manager_t* sensor_ad_get_instance(void);

/**
 * @brief Checks if the sensor auto-detect manager is initialized.
 *
 * @return true if initialized, false otherwise
 */
bool sensor_ad_is_initialized(void);

sensor_system_t* sensor_ctl_get_instance(void);

// 안전한 접근 함수들
void my_manager_set_data(int val);
int my_manager_get_data(void);
void my_manager_add_data(int delta);

#ifdef __cplusplus
}
#endif

#endif  // SENSOR_AUTO_DETECT_H
