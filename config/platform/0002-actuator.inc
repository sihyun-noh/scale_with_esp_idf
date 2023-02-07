config BS_PLATFORM_ACTUATORS
  bool "Actuators"
  select LED_PACKAGE
  select ETHERNET_PACKAGE
  select LWMQTT_ENABLED
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
  select WIFILIB_PACKAGE
  select FILELOG_FEATURE
  select MONITOR_FEATURE
  select OTA_FEATURE
  select EASYSETUP_FEATURE

  help
    Actutactors to control the slide windows and other stuff

  choice
    prompt "Actuator Type"
    depends on BS_PLATFORM_ACTUATORS
    help
      Select actuator type

    config ACTUATOR_SWITCH
      bool "SWITCH"
      help
        On/Off Type actuator

    config ACTUATOR_MOTOR
      bool "MOTOR"
      help
        Forward/Reverse Type actuator

  endchoice
