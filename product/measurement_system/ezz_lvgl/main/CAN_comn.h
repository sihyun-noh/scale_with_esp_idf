#ifndef TWAI_PORT_H_
#define TWAI_PORT_H_

#include <stdio.h>              // Standard input/output library
#include <stdlib.h>             // Standard library for memory allocation, etc.
#include "freertos/FreeRTOS.h"  // FreeRTOS header
#include "freertos/task.h"      // FreeRTOS task management
#include "freertos/queue.h"     // FreeRTOS queue management
#include "freertos/semphr.h"    // FreeRTOS semaphore management
#include "sdkconfig.h"
#include "esp_err.h"            // ESP-IDF error codes
#include "esp_log.h"            // ESP-IDF logging library
#include "driver/twai.h"        // TWAI driver header
#include <esp_timer.h>          // ESP timer library

#include "driver/i2c.h"   // I2C driver header
#include "driver/gpio.h"  // GPIO driver header

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_MASTER_SCL_IO 9 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 8 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ        400000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0      /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0      /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS     1000

/* --------------------- Definitions and static variables ------------------ */
// Example Configuration
// #define TX_GPIO_NUM CONFIG_EXAMPLE_TX_GPIO_NUM  // Transmit GPIO number
// #define RX_GPIO_NUM CONFIG_EXAMPLE_RX_GPIO_NUM  // Receive GPIO number

#if defined(CONFIG_TARGET_WAVESHARE_4_3)

#define TX_GPIO_NUM 20
#define RX_GPIO_NUM 19

#elif defined(CONFIG_TARGET_WAVESHARE_5)

#define TX_GPIO_NUM 15
#define RX_GPIO_NUM 16

#else
#error "No target selected in menuconfig"
#endif

// Intervals:
#define TRANSMIT_RATE_MS 10    // Transmission interval in milliseconds
#define POLLING_RATE_MS  1000  // Polling interval in milliseconds

#define CAN_FRAME_STANDARD 0
#define CAN_FRAME_EXTENDED 1

#ifndef CAN_FRAME_FORMAT
#define CAN_FRAME_FORMAT CAN_FRAME_EXTENDED
#endif

#define CAN_RX_FILTER_ENABLE 1

#define CAN_RX_FILTER_SIBI_ID_1 0x00000001UL
#define CAN_RX_FILTER_SIBI_ID_2 0x00000002UL

#define CAN_RX_FILTER_UPPER_STATUS_ID     0x18FF0310UL
#define CAN_RX_FILTER_UPPER_STATUS_RPM_ID 0x18FF0300UL

esp_err_t waveshare_twai_init();                                    // Function to initialize TWAI
esp_err_t waveshare_twai_receive(twai_message_t *msg);              // Function to receive TWAI messages
esp_err_t waveshare_twai_transmit(int user_data, uint32_t can_id);  // Transmit data via TWAI
esp_err_t evt_twai_transmit(uint8_t *payload, uint32_t can_id);

#ifdef __cplusplus
}
#endif
#endif
