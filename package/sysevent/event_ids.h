#ifndef _EVENT_IDS_H_
#define _EVENT_IDS_H_

enum {
  I2C_TEMPERATURE_EVENT = 1,
  I2C_HUMIDITY_EVENT = 2,
  ADC_BATTERY_EVENT = 3,
  I2C_CO2_EVENT = 4,
  EASY_SETUP_DONE = 5,
  MDNS_EVENT = 6,
  MDNS_START = 7,
  MDNS_STOP = 8,
  NO_NETWORK_EVENT = 9,
  OK_NETWORK_EVENT = 10,
  NO_INTERNET_EVENT = 11,
  OK_INTERNET_EVENT = 12,
  HEAP_CRITICAL_LEVEL_WARNING_EVENT = 13,
  HEAP_WARNING_LEVEL_WARNING_EVENT = 14,
  TASK_MONITOR_EVENT = 15,
  MB_TEMPERATURE_EVENT = 16,
  MB_MOISTURE_EVENT = 17,
  MB_SOIL_EC_EVENT = 18,
  MB_PYRANOMETER_EVENT = 19,
  I2C_PH_EVENT = 20,
  I2C_EC_EVENT = 21,
  MB_WATER_PH_EVENT = 22,
  MB_SOIL_BULK_EC_EVENT = 23,
  MB_WIND_DIRECTION_EVENT = 24,
  MB_WIND_SPEED_EVENT = 25,
  MB_WATER_EC_EVENT = 26,
  ACTUATOR_AUTO = 50,
  ACTUATOR_PORT1 = 51,
  ACTUATOR_PORT2 = 52,
  ACTUATOR_PORT3 = 53,
  ACTUATOR_PORT4 = 54,
  ACTUATOR_PORT5 = 55,
  ACTUATOR_PORT6 = 56,
  ACTUATOR_PORT7 = 57,
  ACTUATOR_PORT8 = 58,
  WEIGHT_DATA_EVENT = 59,
  WEIGHT_DATA_RES_EVENT = 60,
  WEIGHT_PRINTER_EVENT = 61,
};

#define ACTUATOR_BASE 50

#endif /* _EVENT_IDS_H_ */
