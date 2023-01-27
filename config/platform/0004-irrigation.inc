config BS_PLATFORM_IRRIGATION
  bool "Irrigation"
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
      depends on FILELOG_FEATURE
      help
        Irrigation HID

    config IRRIGATION_MASTER
      bool "MASTER"
      help
        Irrigation master

  endchoice
