#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board : ESP32S3
 *****************************************/
#define MASTER 1
#define HID    2
#define CHILD  3

#define IRRIGATION_TYPE HID

#define SENSOR_TYPE     -1
#define ACTUATOR_TYPE   -1

#define BATTERY_PORT 6  // GPIO34

#define SDCARD_SPI_MISO 2
#define SDCARD_SPI_MOSI 15
#define SDCARD_SPI_CLK  14
#define SDCARD_SPI_CS   13

#define SDCARD_SPI_DMA_CHAN 1

#endif /* _CONFIG_H_ */
