# Welcome to the smartfarm-ief (SmartFarm IoT Eco Framework) wiki!

This framework is designed to act as a middleware between the hardware and the application layer so that application layer uses the same APIs regardless of the SDK of hardware manufacturer used in the product.

Currently, we are using espressif's esp32 hardware, but our goal is to create a framework that can operate on various hardware with minimal porting even if other hardware is used in the future.

## Framework Overview
There may be situations where we are working to release multiple products simultaneously for each product purpose.

Working on many products at the same time leads to a lot of code redundancy and all changes (improvements/bug fixes) made in one branch need to be propagated to all other product branches.(we don't want to do the same thing over and over for all products to improve or fix some issues.)

That's why we introduced the SmartFarm framework to improve this situation.

The basic idea is to eliminate this redundancy by having certain "product-independent" core components and others, which are product-specific. This will help maintaining the code across products/projects in a better way.

To start with, the core components employed in Sensors :
* Syscfg : Refer to [[Syscfg]]
  * Manufacturing Data : [[Manufacturing Data]]
* Sysevent : [[Sysevent]]
* Interactive command shell
* EasySetup
* WiFi library
* PAL (Peripheral Abstraction Layer)
* Logging 
* Monitoring
* Network Protocol
  * MQTT
  * mDNS
  * HTTPS
* Sensors
  * SHT3x - Temperature and Humidity
  * SCD4x - CO2 and Temperature/Humidity
  * Battery module
* Unit Test
  * Please refer to the following [[Unit Test]] for more detailed information.

## Compoenents of Framework
+ Console command-line interface
  + Interactive command-line in console mode
  + Provides commands for debugging purposes
+ Configuration management
  + syscfg : Module that can set/get the system configuration of device in non-volatile storage area.
    + Configuration data of device
    + Manufacturing data of device
+ Event Messaging management
  + sysevent : Module that can trigger and receive an event between tasks.
    + Trgger/Receive an event and value between tasks
    + Register an event handler that is called when a specific event is received.
+ Network manaegemt
  + EasySetup : Zero touch setup without user intervention.
  + WiFi Management : Network recovery mechanism when wifi connections is not stable.
  + Supports other wireless technologies such as BLE, Zigbee, etc. (if needed)
+ Multi protocols
  + MQTT
  + HTTP/S
  + COAP
  + mDNS
  + UPnP/SSDP
  + ICMP Echo
  + TLS/OpenSSL
  + Modbus RTU (RS232, RS422, RS485) (if needed)
+ PAL(HAL) : Peripheral Abstraction Layer
  + ADC
  + DAC
  + I2C
  + SPI
  + UART
  + GPIO
  + PWM
  + LCD/LED (if needed)
+ Log management
  + Local log management (Console log message)
  + Remote log management (Send device log message to server)
+ OTA FW management
  + OTA FW update (Device <-> FW update server)
  + Auto recovery mechanism when fw update is failing.
+ Device management
  + Local command for device control
  + Supports timezone
  + Device grouping
+ Remote management
  + Register device information and capability to the Cloud server
  + Report status and value of device to the server
  + Command control through the server (Receiving a command from server and control it)
  + Device FW version management via server
+ Device(System) monitoring
  + Network connection checking
  + Device health checking
  + Task monitoring
+ Watchdog system
+ Security and Authentication management
  + Data protection and authentication data is used to communicate with Cloud server.
+ Sensor module
  + Temperature/Humidity
  + Co2
  + Weather Station
  + Water PH
  + Water EC
  + Soil Temperature/Humidity/EC
+ Unit Test
  + Running unit test on target system
  
