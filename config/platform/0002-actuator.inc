config BS_PLATFORM_ACTUATORS
  bool "Actuators"
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
