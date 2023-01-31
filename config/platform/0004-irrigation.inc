config BS_PLATFORM_IRRIGATION
  bool "Irrigation"
  select LED_PACKAGE
  select ESPNOW_PACKAGE
  select GPIO_PACKAGE
  select I2C_PACKAGE
  select ADC_PACKAGE
  select PING_PACKAGE
  select SHELL_PACKAGE
  select SYSCFG_PACKAGE
  select SYSEVENT_PACKAGE
  select SYSLOG_PACKAGE
  select TIMEZONE_PACKAGE
  select UTILS_PACKAGE
  select WIFILIB_PACKAGE

  help
    Irrigation system controls the water flow using user input conditions.

  choice
    prompt "Irrigation Type"
    depends on BS_PLATFORM_IRRIGATION
    help
      Select irrigation type

    config IRRIGATION_CHILD
      bool "CHILD"
      help
        Irrigation child

    config IRRIGATION_HID
      bool "HID"
      select FILELOG_FEATURE
      help
        Irrigation HID

    config IRRIGATION_MASTER
      bool "MASTER"
      select RTC_DS3231_PACKAGE
      help
        Irrigation master

  endchoice
