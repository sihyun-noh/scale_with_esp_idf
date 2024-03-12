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

#define LCD_GPIO_1 GPIO_NUM_10
#define LCD_GPIO_2 GPIO_NUM_11
#define LCD_GPIO_3 GPIO_NUM_12
#define LCD_GPIO_4 GPIO_NUM_13
#define LCD_GPIO_5 GPIO_NUM_14
#define LCD_GPIO_6 GPIO_NUM_21

#define SDCARD_SPI_DMA_CHAN SPI_DMA_CH_AUTO

#define PROD_NUM 30
#define MAX_DATA_LEN 80

#if (CONFIG_MS_RS485_MODBUS)
#define MB_RX_PIN 1
#define MB_TX_PIN 42
#define RTS_UNCHANGED 2
#define CTS_UNCHANGED (-1)
#define UART_PORT_NUM 2
#define BAUD_RATE 9600
#define UART_RXBUF_SIZE 2048
#define UART_TXBUF_SIZE 0

#define CAS_INDICATOR_MODEL 1
#define BAYKON_INDICATOR_BX11 0
#endif

#if (CONFIG_LITTLEFS_PACKAGE)
#define SEN_DATA_PATH_1 "/storage"
#define SEN_DATA_PATH_2 "/storage/Sen2"
#define SEN_DATA_PATH_3 "/storage/Sen3"
#else
#define SEN_DATA_PATH_1 "/spiffs/Sen1"
#define SEN_DATA_PATH_2 "/spiffs/Sen2"
#define SEN_DATA_PATH_3 "/spiffs/Sen3"
#endif

#endif /* _CONFIG_H_ */
