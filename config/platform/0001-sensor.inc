config BS_PLATFROM_SENSORS
  bool "Sensors"
  select MONITOR_FEATURE
  select EASYSETUP_FEATURE
  select OTA_FEATURE

  help
    Sensor node supports several sensors type such as Air, Soil etc
