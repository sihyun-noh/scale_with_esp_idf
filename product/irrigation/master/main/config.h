#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board version
 *****************************************/
#define SENS_OLIMEX_HW 10

#define SENS_TTGO_HW1 11
#define SENS_TTGO_HW3 (SENS_TTGO_HW1 + 2)

#define SENS_GLS_HW 20 /* Greenlabs HW */

#define SENS_BOARD_VER SENS_GLS_HW
// #define SENS_BOARD_VER SENS_OLIMEX_HW

#define MASTER 1
#define HID    2
#define CHILD  3

#define IRRIGATION_TYPE MASTER

#define SENSOR_TYPE     -1
#define ACTUATOR_TYPE   -1

#define BATTERY_PORT 6  // GPIO34

#define LED_RED 33
#define LED_GREEN 26
#define LED_BLUE 25

/*****************************************
 * Config
 ****************************************/
#define MQTT_SEND_INTERVAL 30

/*****************************************
 * Task Priority / Stack size
 ****************************************/
#define SENS_MONITOR_TASK_PRIORITY (tskIDLE_PRIORITY + 5)
#define SENS_SETUP_TASK_PRIORITY (tskIDLE_PRIORITY + 5)

#define SENS_MONITOR_TASK_STACK_SIZE (4096)
#define SENS_SETUP_TASK_STACK_SIZE (4096)

#endif /* _CONFIG_H_ */
