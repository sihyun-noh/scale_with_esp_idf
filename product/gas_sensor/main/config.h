#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board version
 *****************************************/
#define MB_RX_PIN 8
#define MB_TX_PIN 9
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 4800

#define BATTERY_ADC_CHANNEL  3  // GPIO_4
#define BATTERY_READ_ON_GPIO 40 // GPIO_40

// esp32s3  datalogger_v2 rs485 power control pin
#define SENSOR_POWER_CONTROL_PORT 41  // GPIO41

#endif /* _CONFIG_H_ */
