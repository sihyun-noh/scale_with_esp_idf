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

#define BATTERY_ADC_CHANNEL  6  // GPIO_34
#define BATTERY_READ_ON_GPIO 5  // GPIO_5

#define SOL_PW  21
#define SOL_ON  22

#define FUNC_KEY1  14
#define FUNC_KEY2  13

#define LED_RED   27
#define LED_GREEN 25
#define LED_BLUE  26

#endif /* _CONFIG_H_ */
