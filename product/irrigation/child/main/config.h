#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board : ESP32
 *****************************************/
#define MASTER 1
#define HID    2
#define CHILD  3

#define IRRIGATION_TYPE CHILD

#define SENSOR_TYPE     -1
#define ACTUATOR_TYPE   -1

#define BATTERY_PORT 6  // GPIO34

#define SOL_PW  21
#define SOL_ON  22

#define FUNC_KEY1  12
#define FUNC_KEY2  13

#define LED_RED   27
#define LED_GREEN 26
#define LED_BLUE  25

#endif /* _CONFIG_H_ */
