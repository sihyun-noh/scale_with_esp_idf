#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board version
 *****************************************/
#define ACT_TTGO_HW1 0
#define ACT_TTGO_HW3 (ACT_TTGO_HW1 + 2)
#define ACT_OLIMEX_HW 10

#define ACT_GLS_HW 20 /* Greenlabs HW */

#define ACT_BOARD_VER ACT_OLIMEX_HW

#define SWITCH 1 /* ON / OFF type */
#define MOTOR 2  /* forward and reverse motor type */

#ifndef ACTUATOR_TYPE
#define ACTUATOR_TYPE SWITCH
#endif

#if defined(ACT_BOARD_VER) && (ACT_BOARD_VER == ACT_GLS_HW)

#elif defined(ACT_BOARD_VER) && (ACT_BOARD_VER == ACT_OLIMEX_HW)
#define PIN_PHY_POWER 12
#endif

/*****************************************
 * Config
 ****************************************/

/*****************************************
 * Task Priority / Stack size
 ****************************************/
#define ACT_MONITOR_TASK_PRIORITY (tskIDLE_PRIORITY + 5)
#define ACT_MQTT_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define ACT_SETUP_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define ACT_OTAFW_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define ACT_ACTUATOR_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

#define ACT_MONITOR_TASK_STACK_SIZE (4096)
#define ACT_MQTT_TASK_STACK_SIZE (4096)
#define ACT_SETUP_TASK_STACK_SIZE (4096)
#define ACT_OTAFW_TASK_STACK_SIZE (4096)
#define ACT_ACTUATOR_TASK_STACK_SIZE (4096)

#endif /* _CONFIG_H_ */
