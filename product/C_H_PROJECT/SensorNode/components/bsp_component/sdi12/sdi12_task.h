
#ifndef SDI12_OPER_H
#define SDI12_OPER_H

#include "stdio.h"
#include "sensor_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define SDI12_DEBUG_SET

typedef enum { SDI_CMD_INFO = 0, SDI_CMD_ADDR, SDI_CMD_READ, SDI_CMD_M, SDI_CMD_D, SDI_CMD_COUNT } sdi12_cmd_type_t;

typedef struct {
  const char *cmd_format;  // "I!", "R0!"
  int wait_delay_ms;       // Delay time
} sdi12_cmd_template_t;

typedef struct {
  char address;
  float vwc;
  float temperature;
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

typedef struct {
  char address;
  float vwc1;
  float temp1;
  float vwc2;
  float temp2;
  float vwc3;
  float temp3;
  float vwc4;
  float temp4;
} teros54_data_t;

// ATMOS14

typedef struct {
  char address;
  float vaporPressure;
  float temperature;
  float relativeHumidity;
  float atmosphericPressure;
} weather_atmos14_data_t;

// ATMOS22
typedef struct {
  char address;
  float windSpeed;
  float windDirection;
  float gustWindSpeed;
  float airTemperature;
  float xOrientation;
  float yOrientation;
  float nullValue;
  float northWindSpeed;
  float eastWindSpeed;
} weather_atmos22_data_t;

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
  char address;
  float depth;
  float temperature;
  float ec;
} hydros21_data_t;

typedef struct {
  char address;
  unsigned type;
  float PPFD;
  float NDVI_UP_WARD;
  float NDVI_DOWN_WARD;
} apogee_sensor_t;

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
 * @brief Reads measurement data from a TEROS11 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses the result into a teros11_data_t structure containing
 * volumetric water content (VWC) and temperature.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated teros11_data_t structure with parsed data.
 */
teros11_data_t *sdi12_read_teros11(uint8_t portId);

/**
 * @brief Reads measurement data from a TEROS12 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a teros12_data_t structure containing volumetric water content (VWC),
 * temperature, and electrical conductivity (EC).
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated teros12_data_t structure with parsed data.
 */
teros12_data_t *sdi12_read_teros12(uint8_t portId);

/**
 * @brief Reads measurement data from a TEROS21 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a teros21_data_t structure containing matric potential and temperature values.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated teros21_data_t structure with parsed data.
 */
teros21_data_t *sdi12_read_teros21(uint8_t portId);

/**
 * @brief Reads measurement data from an ATMOS41 (AT41G2) sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a weather_at41g2_data_t structure containing weather parameters.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated weather_at41g2_data_t structure with parsed data.
 */
weather_at41g2_data_t *sdi12_read_atmos41(uint8_t portId);

/**
 * @brief Reads measurement data from a TEROS54 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a teros54_data_t structure containing measurements from
 * four positions: VWC1, Temp1, VWC2, Temp2, VWC3, Temp3, VWC4, Temp4.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated teros54_data_t structure with parsed data.
 */
teros54_data_t *sdi12_read_teros54(uint8_t portId);

/**
 * @brief Reads measurement data from an ATMOS22 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a weather_atmos22_data_t structure containing weather parameters.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated weather_atmos22_data_t structure with parsed data.
 */
weather_atmos22_data_t *sdi12_read_atmos22(uint8_t portId);

/**
 * @brief Reads measurement data from an ATMOS14 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a weather_atmos14_data_t structure containing weather parameters.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated weather_atmos14_data_t structure with parsed data.
 */
weather_atmos14_data_t *sdi12_read_atmos14(uint8_t portId);

/**
 * @brief Reads measurement data from an HYDROS21 sensor using the "R0!" SDI-12 command.
 *
 * Sends the "R0!" command to the specified port, waits for the response,
 * and parses it into a hydros21_data_t structure containing volumetric water depth,
 * temperature, and electrical conductivity (EC).
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated hydros21_data_t structure with parsed data.
 */
hydros21_data_t *sdi12_read_hydros21(uint8_t portId);

// /**
//  * @brief Reads measurement data from an Apogee S2-412 sensor using "M!" and "D!" SDI-12 commands.
//  *
//  * Issues the "M!" command to initiate measurement and then "D!" to retrieve the result,
//  * parsing it into an apogee_sensor_t structure containing NDVI (Normalized Difference Vegetation Index) data.
//  *
//  * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
//  * @return Pointer to a statically allocated apogee_sensor_t structure with parsed NDVI data.
//  */
// apogee_sensor_t *sdi12_read_S2_412(uint8_t portId);
//

/**
 * @brief Reads measurement data from an Apogee S2-411 or S2-412 NDVI sensor using "M!" and "D!" SDI-12 commands.
 *
 * Issues the "M!" command to initiate measurement and then "D!" to retrieve the result,
 * parsing it into an `apogee_sensor_t` structure containing NDVI (Normalized Difference Vegetation Index) data.
 *
 * - **S2-411**: Upward-facing sensor (sky-facing), typically installed pointing toward the sky.
 * - **S2-412**: Downward-facing sensor (canopy-facing), typically installed facing vegetation.
 *
 * These sensors are typically always powered on or initialized once at boot and do not support
 * dynamic power control via GPIO. A delay is included after power initialization to allow for proper boot-up.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @param type Sensor type: APOGEE_S2_411 or APOGEE_S2_412
 * @return Pointer to a statically allocated `apogee_sensor_t` structure with parsed NDVI data,
 *         or NULL if parsing fails.
 */

apogee_sensor_t *sdi12_read_S2_411_412(uint8_t portId, sdi12_sensor_type_t type);

/**
 * @brief Reads measurement data from an Apogee SQ-521 sensor using "M!" and "D!" SDI-12 commands.
 *
 * Issues the "M!" command to initiate measurement and then "D!" to retrieve the result,
 * parsing it into an apogee_sensor_t structure containing PPFD (Photosynthetic Photon Flux Density) data.
 *
 * @param portId Sensor port ID (0–N) selecting the SDI-12 bus or multiplexer channel.
 * @return Pointer to a statically allocated apogee_sensor_t structure with parsed PPFD data.
 */
apogee_sensor_t *sdi12_read_SQ_521(uint8_t portId);

/**
 * @brief clean up buffer
 *
 */
char *trim(char *str);

/**
 * @brief Test task
 *
 */
void sdi12_task_init(void);

#ifdef __cplusplus
}
#endif
#endif
