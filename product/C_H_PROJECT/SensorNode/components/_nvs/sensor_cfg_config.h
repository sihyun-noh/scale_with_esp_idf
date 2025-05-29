
#ifndef SENSOR_CFG_CONFIG_H
#define SENSOR_CFG_CONFIG_H

/**
 * @file sensor_cfg_config.h
 * @brief Configuration interface for managing individual sensor port settings.
 *
 * This module provides APIs to load, save, and initialize configuration data
 * for each of the 6 sensor ports using the 'cfg' NVS partition. Each port can store
 * structured settings such as enable flags, operation mode, and threshold values.
 *
 * The configuration is persisted across reboots and can be updated dynamically
 * via MQTT or CLI commands.
 */

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_PORT_COUNT 6

/// Get the CFG key name for a given port (e.g., "sensor_port_1")
#define SENSOR_PORT_KEY(i) "sensor_port_" #i

typedef enum {
  TEROS11 = 0x00,
  TEROS12,
  TEROS14,
  TEROS21,
  ATMOS21,
  ATMOS22,
  ATMOS31,
  ATMOS41,
  ATMOS54,
  APOGEE_S2_411,
  APOGEE_SP_421,
  APOGEE_SQ_521,
  APOGEE_SU_221,
  SENSOR_TYPE_COUNT,
} sdi12_sensor_type_t;

/// Structure to hold configuration for a single sensor port
typedef struct {
  uint8_t port;
  uint8_t enabled;                  ///< 1 if port is active, 0 if disabled
  sdi12_sensor_type_t sensor_type;  ///< Sensor type configuration
  uint16_t threshold;               ///< Threshold value for triggering actions
  uint8_t dirty;                    ///< 1 if modified, 0 if clean
  uint8_t sampling_period_sec;
  uint8_t processing_interval_period_min;
  uint8_t columns_size;
  uint8_t rows_size;
} sensor_port_cfg_t;

/**
 * @brief Load sensor port configuration from NVS.
 *
 * @param port Port number (1–6)
 * @param[out] cfg Pointer to sensor_port_cfg_t structure to populate
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if port is out of range
 *      - ESP_ERR_NVS_NOT_FOUND if no data is stored
 *      - other esp_err_t codes from NVS
 */
esp_err_t sensor_port_cfg_load(int port, sensor_port_cfg_t *cfg);

/**
 * @brief Save sensor port configuration to NVS.
 *
 * @param port Port number (1–6)
 * @param[in] cfg Pointer to sensor_port_cfg_t structure to save
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if port is out of range
 *      - other esp_err_t codes from NVS
 */
esp_err_t sensor_port_cfg_save(int port, const sensor_port_cfg_t *cfg);

/**
 * @brief Initialize sensor port configuration with default values if not present.
 *
 * Loads configuration for all sensor ports. If a port does not have saved config,
 * default values are written to NVS.
 *
 * @return
 *      - ESP_OK if all ports were loaded or initialized successfully
 *      - other esp_err_t on error
 */
esp_err_t sensor_port_cfg_init(void);

/**
 * @brief Get pointer to configuration for a specific sensor port.
 *
 * @param port Port number (1~6)
 * @return Pointer to sensor_port_cfg_t, or NULL if out of range
 */
sensor_port_cfg_t *sensor_port_cfg_get(int port);

/// Get pointer to runtime configuration array (indexed 0~5)
sensor_port_cfg_t *sensor_cfg_instance(void);

/// Save updated ports (dirty only) to NVS
esp_err_t sensor_port_cfg_commit(void);

/**
 * @brief Update sensor port configuration from JSON payload.
 *
 * Parses the provided JSON string and updates the corresponding fields
 * in the given sensor_port_cfg_t structure (enabled, threshold, mode).
 * If any field is updated, the `dirty` flag is set to 1.
 *
 * Example JSON:
 * {
 *   "enabled": 1,
 *   "threshold": 150,
 *   "mode": 2
 * }
 *
 * @param[in,out] cfg Pointer to the sensor port configuration to update.
 * @param[in] json_payload Null-terminated JSON string received via MQTT.
 *
 * @return void
 */
// void handle_mqtt_config_update(sensor_port_cfg_t *cfg, const char *json_payload);

/**
 * @brief Handle incoming MQTT configuration JSON and update sensor port settings.
 *
 * Parses the given JSON payload and updates the corresponding sensor port configuration,
 * including fields such as `enabled`, `threshold`, and `mode`. The configuration is marked
 * as dirty if any value is changed, indicating that it needs to be committed later.
 *
 * Expected JSON format:
 * {
 *   "port": 1,
 *   "enabled": 1,
 *   "threshold": 30,
 *   "mode": 2
 * }
 *
 * @param json_payload A null-terminated JSON string containing configuration values.
 * @return ESP_OK on success, or an error code if parsing fails or values are invalid.
 */
esp_err_t handle_mqtt_config_update(const char *json_payload);

/**
 * @brief Initialize and store device_id in cfg if not already present.
 *
 * @return ESP_OK on success, or error code
 */
esp_err_t cfg_init_device_id(void);

/**
 * @brief Get current device_id from cfg.
 *
 * @param[out] out_id Caller-owned pointer. Must be freed by caller.
 * @return ESP_OK or error
 */
esp_err_t get_device_id(char **out_id);

#ifdef __cplusplus
}
#endif
#endif  // SENSOR_CFG_CONFIG_H
