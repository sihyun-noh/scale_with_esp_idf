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

#define SENSOR_TYPE -1

#define BATTERY_PORT 6  // GPIO34

#define LED_RED 25
#define LED_GREEN 26
#define LED_BLUE 12

#define SDCARD_SPI_MISO 2
#define SDCARD_SPI_MOSI 15
#define SDCARD_SPI_CLK  14
#define SDCARD_SPI_CS   13

#define SDCARD_SPI_DMA_CHAN 1

#endif /* _CONFIG_H_ */
