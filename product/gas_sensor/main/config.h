#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board version
 *****************************************/
#if (CONFIG_DATALOGGER_SHT3X)
#define SHT3X_I2C_SDA_PIN 1
#define SHT3X_I2C_SCL_PIN 2
#elif (CONFIG_DATALOGGER_SCD4X)
#define SCD4X_I2C_SDA_PIN 21
#define SCD4X_I2C_SCL_PIN 22
#elif (CONFIG_DATALOGGER_ATLAS_PH)
#define ATLAS_PH_I2C_SDA_PIN 21
#define ATLAS_PH_I2C_SCL_PIN 22
#elif (CONFIG_DATALOGGER_ATLAS_EC)
#define ATLAS_EC_I2C_SDA_PIN 21
#define ATLAS_EC_I2C_SCL_PIN 22
#elif (CONFIG_DATALOGGER_RK520_02 || CONFIG_DATALOGGER_RK500_02 || CONFIG_DATALOGGER_RK100_02 || CONFIG_DATALOGGER_RK110_02 || \
       CONFIG_DATALOGGER_RK500_13)
// Greenlabs Board PIN numbers for U2RX and U2TX
#define MB_RX_PIN 16
#define MB_TX_PIN 17
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 9600

#elif (CONFIG_DATALOGGER_RS_ECTH)
#define MB_RX_PIN 8
#define MB_TX_PIN 9
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 4800

#elif (CONFIG_DATALOGGER_SWSR7500)
#define MB_RX_PIN 16
#define MB_TX_PIN 17
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 38400
#endif

#define BATTERY_ADC_CHANNEL  3  // GPIO_4
#define BATTERY_READ_ON_GPIO 40 // GPIO_40

// esp32s3  datalogger_v2 rs485 power control pin
#define SENSOR_POWER_CONTROL_PORT 41  // GPIO41

#define LED_RED   25
#define LED_GREEN 26
#define LED_BLUE  12

#endif /* _CONFIG_H_ */
