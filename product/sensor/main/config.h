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
//#define SENS_BOARD_VER SENS_OLIMEX_HW

#define SHT3X 1    /* Temperature and Humidity Sensor */
#define SCD4X 2    /* CO2 and Temperature and Humidity */
#define RK520_02 3 /* Soil EC Rika Sensor */
#define SWSR7500 4 /* KD Solar Radiation */

#ifndef SENSOR_TYPE
#define SENSOR_TYPE SHT3X
#endif

#if defined(SENS_BOARD_VER) && (SENS_BOARD_VER >= SENS_TTGO_HW1)
#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SHT3X)
#define SHT3X_I2C_SDA_PIN 21
#define SHT3X_I2C_SCL_PIN 22
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == SCD4X)
#define SCD4X_I2C_SDA_PIN 21
#define SCD4X_I2C_SCL_PIN 22
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == RK520_02)
// Greenlabs Board PIN numbers for U2RX and U2TX
#define MB_RX_PIN 16
#define MB_TX_PIN 17
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 9600
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == SWSR7500)
#define MB_RX_PIN 16
#define MB_TX_PIN 17
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 38400
#endif
#elif defined(SENS_BOARD_VER) && (SENS_BOARD_VER == SENS_OLIMEX_HW)
#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SHT3X)
#define SHT3X_I2C_SDA_PIN 13
#define SHT3X_I2C_SCL_PIN 16
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == SCD4X)
#define SCD4X_I2C_SDA_PIN 13
#define SCD4X_I2C_SCL_PIN 16
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == RK520_02)
// Olimex Board PIN numbers for U1RX and U1TX
#define MB_RX_PIN 36
#define MB_TX_PIN 4
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 1
#endif
#endif

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
#define SENS_MQTT_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define SENS_SETUP_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define SENS_OTAFW_TASK_PRIORITY (tskIDLE_PRIORITY + 3)

#define SENS_MONITOR_TASK_STACK_SIZE (4096)
#define SENS_MQTT_TASK_STACK_SIZE (4096)
#define SENS_SETUP_TASK_STACK_SIZE (4096)
#define SENS_OTAFW_TASK_STACK_SIZE (4096)

#endif /* _CONFIG_H_ */
