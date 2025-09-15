
#include "sensor_config.h"

/* clang-format off */
const char *sensor_names[] = {
  "INFO", 
  "TEROS11", 
  "TEROS12",
  // "TEROS14",
  "TEROS21",
  "TEROS54",
  "ATMOS14",
  // "ATMOS21",
   "ATMOS22",
  //  "ATMOS31",
  "ATMOS41", 
  "APO_S2_411", 
  "APO_S2_412", 
  //"APOGEE_SP_421", 
  "APO_SQ_521",
  //"APOGEE_SU_221",  // ...
};

/* clang-format on */
const char *sensor_type_to_str(sdi12_sensor_type_t type) {
  if (type >= 0 && type < SENSOR_TYPE_COUNT) {
    return sensor_names[type];
  }
  return "UNKNOWN";
}
