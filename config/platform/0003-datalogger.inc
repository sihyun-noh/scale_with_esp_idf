config BS_PLATFORM_DATALOGGER
  bool "DataLogger"
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
    DataLogger gathers the log data of sensor

  choice
    prompt "Datalogger Type"
    depends on BS_PLATFORM_DATALOGGER
    help
      Select data logger type

    config DATALOGGER_SHT3X
      bool "SHT3X"
      help
        Sensirion SHT3x Temperature and Humidity sensor

    config DATALOGGER_SCD4X
      bool "SCD4X"
      help
        Sensirion SCD4x Co2, temperature and humidity sensor

    config DATALOGGER_RK520_02
      bool "RK520_02"
      help
        Rika Soil EC sensor

    config DATALOGGER_SWSR7500
      bool "SWSR7500"
      help
        KD Solar Radiation sensor

    config DATALOGGER_ATLAS_PH
      bool "ATLAS_PH"
      help
        Atlas pH sensor

    config DATALOGGER_ATLAS_EC
      bool "ATLAS_EC"
      help
        Atlas EC sensor

    config DATALOGGER_RK500_02
      bool "RK500_02"
      help
        Rika Water pH sensor

    config DATALOGGER_RK110_02
      bool "RK110_02"
      help
        Rika Wind Direction sensor

    config DATALOGGER_RK100_02
      bool "RK100_02"
      help
        Rika Wind Speed sensor

    config DATALOGGER_RK500_13
      bool "RK500_13"
      help
        Rika Water EC sensor

    config DATALOGGER_RS_ECTH
      bool "RS_ECTH"
      help
        Renke Soil EC sensor

  endchoice
