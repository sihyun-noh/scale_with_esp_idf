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

#define SHT3X 1     /* Temperature and Humidity Sensor */
#define SCD4X 2     /* CO2 and Temperature and Humidity */
#define RK520_02 3  /* Soil EC Rika Sensor */
#define SWSR7500 4  /* KD Solar Radiation */
#define ATLAS_PH 5  /* Atlas pH Sensor */
#define ATLAS_EC 6  /* Atlas EC Sensor */
#define RK500_02 7  /* Water PH Rika Sensor */
#define RK110_02 8  /* Wind Direction Rika Sensor */
#define RK100_02 9  /* Wind Speed Rika Sensor */
#define RK500_13 10 /* Water EC Rika Sensor */

#ifndef SENSOR_TYPE
#define SENSOR_TYPE SHT3X
#endif

#define ACTUATOR_TYPE -1

#define DS3231_I2C_SDA_PIN 33
#define DS3231_I2C_SCL_PIN 32

#if defined(SENS_BOARD_VER) && (SENS_BOARD_VER >= SENS_TTGO_HW1)
#if (SENSOR_TYPE == SHT3X)
#define SHT3X_I2C_SDA_PIN 33
#define SHT3X_I2C_SCL_PIN 32
#elif (SENSOR_TYPE == SCD4X)
#define SCD4X_I2C_SDA_PIN 21
#define SCD4X_I2C_SCL_PIN 22
#elif (SENSOR_TYPE == ATLAS_PH)
#define ATLAS_PH_I2C_SDA_PIN 21
#define ATLAS_PH_I2C_SCL_PIN 22
#elif (SENSOR_TYPE == ATLAS_EC)
#define ATLAS_EC_I2C_SDA_PIN 21
#define ATLAS_EC_I2C_SCL_PIN 22
#elif (SENSOR_TYPE == RK520_02 || SENSOR_TYPE == RK500_02 || SENSOR_TYPE == RK100_02 || SENSOR_TYPE == RK110_02 || \
       SENSOR_TYPE == RK500_13)
// Greenlabs Board PIN numbers for U2RX and U2TX
#define MB_RX_PIN 16
#define MB_TX_PIN 17
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 9600
#elif (SENSOR_TYPE == SWSR7500)
#define MB_RX_PIN 16
#define MB_TX_PIN 17
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 38400
#endif
#elif defined(SENS_BOARD_VER) && (SENS_BOARD_VER == SENS_OLIMEX_HW)
#if (SENSOR_TYPE == SHT3X)
#define SHT3X_I2C_SDA_PIN 13
#define SHT3X_I2C_SCL_PIN 16
#elif (SENSOR_TYPE == SCD4X)
#define SCD4X_I2C_SDA_PIN 13
#define SCD4X_I2C_SCL_PIN 16
#elif (SENSOR_TYPE == ATLAS_PH)
#define ATLAS_PH_I2C_SDA_PIN 13
#define ATLAS_PH_I2C_SCL_PIN 16
#elif (SENSOR_TYPE == ATLAS_EC)
#define ATLAS_EC_I2C_SDA_PIN 13
#define ATLAS_EC_I2C_SCL_PIN 16
#elif (SENSOR_TYPE == RK520_02)
// Olimex Board PIN numbers for U1RX and U1TX
#define MB_RX_PIN 36
#define MB_TX_PIN 4
#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 1
#endif
#endif

#define BATTERY_PORT 6  // GPIO34

#define SDCARD_SPI_MISO 2
#define SDCARD_SPI_MOSI 15
#define SDCARD_SPI_CLK  14
#define SDCARD_SPI_CS   13

#define SDCARD_SPI_DMA_CHAN 1

#endif /* _CONFIG_H_ */
