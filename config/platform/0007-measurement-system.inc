config BS_PLATFORM_MEASUREMENT_SYSTEM
  bool "weight_measurement"
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
  select MODBUS_PACKAGE

  help
    measurement system controls the gross weight judge device.

  choice
    prompt "measurement system Type"
    depends on BS_PLATFORM_MEASUREMENT_SYSTEM
    help
      Select measurement system components.

    config MS_SPEAKER
      bool "MS_SPEAKER"
      help
        SPEAKER ADDON

    config MS_USB_FILE_DOWNLOAD
      bool "MS_USB"
      help
        Use to usb download function

    config MS_RS485_MODBUS
      bool "MS_RS485_MODBUS"
      help
        Use to modbus.

  endchoice
