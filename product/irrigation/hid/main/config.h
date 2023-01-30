#ifndef _CONFIG_H_
#define _CONFIG_H_

/******************************************
 * Board : ESP32S3
 *****************************************/
#define BATTERY_PORT 6  // GPIO34

#define SDSPI_HOST_ID SPI3_HOST
#define SD_MISO GPIO_NUM_38
#define SD_MOSI GPIO_NUM_40
#define SD_SCLK GPIO_NUM_39
#define SD_CS GPIO_NUM_41

#define SDCARD_SPI_DMA_CHAN SPI_DMA_CH_AUTO

#endif /* _CONFIG_H_ */
