config BS_PLATFORM_SENSORS
  bool "Sensors"
  select LED_PACKAGE
  select MODBUS_PACKAGE
  select MQTTC_PACKAGE
  select GPIO_PACKAGE
  select I2C_PACKAGE
  select ADC_PACKAGE
  select PING_PACKAGE
  select SHELL_PACKAGE
  select SYSCFG_PACKAGE
  select SYSEVENT_PACKAGE
  select SYSFILE_PACKAGE
  select SYSLOG_PACKAGE
  select TIMEZONE_PACKAGE
  select UTILS_PACKAGE
  select WIFILIB_PACKAGE
  select FILELOG_FEATURE
  select MONITOR_FEATURE
  select EASYSETUP_FEATURE
  select OTA_FEATURE

  help
    Sensor node supports several sensors type such as Air, Soil etc

  choice
    prompt "Sensor Type"
    depends on BS_PLATFORM_SENSORS
    help
      Select sensor type

    config SENSOR_SHT3X
      bool "SHT3X"
      select SHT3X_FEATURE
      help
        Sensirion SHT3x Temperature and Humidity sensor

    config SENSOR_SCD4X
      bool "SCD4X"
      select SCD4X_FEATURE
      help
        Sensirion SCD4x Co2, temperature and humidity sensor

    config SENSOR_RK520_02
      bool "RK520_02"
      help
        Rika Soil EC sensor

    config SENSOR_SWSR7500
      bool "SWSR7500"
      help
        KD Solar Radiation sensor

    config SENSOR_ATLAS_PH
      bool "ATLAS_PH"
      select ATLAS_PH_FEATURE
      help
        Atlas pH sensor

    config SENSOR_ATLAS_EC
      bool "ATLAS_EC"
      select ATLAS_EC_FEATURE
      help
        Atlas EC sensor

    config SENSOR_RK500_02
      bool "RK500_02"
      help
        Rika Water pH sensor

    config SENSOR_RK110_02
      bool "RK110_02"
      help
        Rika Wind Direction sensor

    config SENSOR_RK100_02
      bool "RK100_02"
      help
        Rika Wind Speed sensor

    config SENSOR_RK500_13
      bool "RK500_13"
      help
        Rika Water EC sensor

    config SENSOR_RS_ECTH
      bool "RS_ECTH"
      help
        Renke Soil EC sensor

  endchoice
