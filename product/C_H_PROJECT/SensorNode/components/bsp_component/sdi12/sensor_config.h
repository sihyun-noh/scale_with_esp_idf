
#ifndef SDI12_CONFIG_H
#define SDI12_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// @brief The sensor_type_to_str function must match the string array exactly
typedef enum {
  INFO_INDEX = 0x00,
  TEROS11,
  TEROS12,
  // TEROS14,
  TEROS21,
  TEROS54,
  // ATMOS21,
  ATMOS22,
  // ATMOS31,
  ATMOS41,
  APOGEE_S2_411,
  APOGEE_S2_412,
  //  APOGEE_SP_421,
  APOGEE_SQ_521,
  // APOGEE_SU_221,
  SENSOR_TYPE_COUNT,
  SENSOR_TYPE_UNKNOWN = 99,
} sdi12_sensor_type_t;

/**
 * @brief Returns the string representation of a given SENSOR_TYPE.
 *
 * @param type The sensor type enum value.
 * @return const char* String name of the sensor type.
 */
const char *sensor_type_to_str(sdi12_sensor_type_t type);

#ifdef __cplusplus
}
#endif
#endif
