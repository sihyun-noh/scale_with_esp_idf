
#ifndef SDI12_OPER_H
#define SDI12_OPER_H

#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define SDI12_DEBUG_SET

typedef enum {
  SDI_CMD_INFO = 0,
  SDI_CMD_ADDR,
  SDI_CMD_READ,
  SDI_CMD_R3,
  SDI_CMD_XO,
  SDI_CMD_COUNT  // 명령 개수
} sdi12_cmd_type_t;

typedef struct {
  const char *cmd_format;  // "I!", "R0!" 등
  int wait_delay_ms;       // 100, 50 등
} sdi12_cmd_template_t;

typedef struct {
  char address;
  float vwc;
  float temperature;
  float ec;
} teros11_data_t;

typedef struct {
  char address;
  float vwc;
  float temperature;
  float ec;
} teros12_data_t;

typedef struct {
  char address;
  float matricPotential;
  float temperature;
} teros21_data_t;

// ATMOS41G2
typedef struct {
  char address;
  float solar;
  float precipitation;
  int strikes;
  int strikeDistance;
  float windSpeed;
  float windDirection;
  float gustWindSpeed;
  float airTemperature;
  float vaporPressure;
  float atmosphericPressure;
  float relativeHumidity;
  float humiditySensorTemperature;
  float xOrientation;
  float yOrientation;
  float nullValue;
  float northWindSpeed;
  float eastWindSpeed;
} weather_at41g2_data_t;

typedef struct {
  char manufacturer[9];  // 8 + null
  char model[9];         // 8 + null
  char version[9];       // 8 + null
  char serial[17];       // 16 + null
} sdi12_sensor_info_t;

/**
 * @brief Requests sensor information using the "I!" SDI-12 command.
 *
 * This function sends the SDI-12 "I!" command to the sensor and parses the
 * response into a structured format including manufacturer, model, version, and serial number.
 * @param portId Sensor port ID (0–N) to select the correct SDI-12 bus or multiplexer channel.
 *
 * @return Pointer to statically allocated sdi12_sensor_info_t structure.
 */
sdi12_sensor_info_t *sdi12_info_start(uint8_t portId);

/**
 * @brief Reads sensor measurement data from a TEROS11 sensor using the "R0!" SDI-12 command.
 *
 * This function sends the SDI-12 "R0!" command to the given port, waits for the sensor to respond,
 * and parses the data into a teros12_data_t structure, including volumetric water content (VWC),
 * temperature
 *
 * @param portId Sensor port ID (0–N) to select the correct SDI-12 bus or multiplexer channel.
 *
 * @return Pointer to statically allocated teros11_data_t structure containing parsed sensor data.
 */
teros11_data_t *sdi12_read_start_teros11(uint8_t portId);

/**
 * @brief Reads sensor measurement data from a TEROS12 sensor using the "R0!" SDI-12 command.
 *
 * This function sends the SDI-12 "R0!" command to the given port, waits for the sensor to respond,
 * and parses the data into a teros12_data_t structure, including volumetric water content (VWC),
 * temperature, and electrical conductivity (EC).
 *
 * @param portId Sensor port ID (0–N) to select the correct SDI-12 bus or multiplexer channel.
 *
 * @return Pointer to statically allocated teros12_data_t structure containing parsed sensor data.
 */
teros12_data_t *sdi12_read_start_teros12(uint8_t portId);

/**
 * @brief Reads data from a TEROS21 sensor connected to the given port.
 *
 * @param portId ID of the sensor port (0–5 typically).
 * @return Pointer to the populated teros21_data_t structure, or NULL on failure.
 */
teros21_data_t *sdi12_read_start_teros21(uint8_t portId);

/**
 * @brief Reads data from an ATMOS41 (AT41G2) weather sensor on the specified port.
 *
 * @param portId ID of the sensor port (0–5 typically).
 * @return Pointer to the populated weather_at41g2_data_t structure, or NULL on failure.
 */
weather_at41g2_data_t *sdi12_read_start_teros41(uint8_t portId);

// clean up buffer
char *trim(char *str);

void sdi12_task_init(void);

#ifdef __cplusplus
}
#endif
#endif
