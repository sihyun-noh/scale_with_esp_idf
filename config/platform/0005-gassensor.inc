config BS_PLATFORM_GASSENSOR
  bool "GasSensor"
  select LED_PACKAGE
  select GPIO_PACKAGE
  select I2C_PACKAGE
  select ADC_PACKAGE
  select PING_PACKAGE
  select RTC_DS3231_PACKAGE
  select SHELL_PACKAGE
  select SYSCFG_PACKAGE
  select SYSEVENT_PACKAGE
  select SYSFILE_PACKAGE
  select SYSLOG_PACKAGE
  select TIMEZONE_PACKAGE
  select UTILS_PACKAGE
  select WIFILIB_PACKAGE
  select FILECOPY_PACKAGE
  select FILELOG_FEATURE
  select USB_HOST_FEATURE

  help
    Gas Sensor gathers the log data of sensor
