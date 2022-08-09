#include <stdio.h>
#include <string.h>

#include "syslog.h"
#include "mb_master_rtu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum {
  SOIL_EC_SENSOR = 0,
  WATER_EC_SENSOR,
  WATER_PH_SENSOR,
  SOLAR_SENSOR,
} MB_SENSOR;

// Olimex Board PIN numbers for U1RX and U1TX
#define MB_RX_PIN 36
#define MB_TX_PIN 4

#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)

#define UART_PORT_NUM 1

const static char *TAG = "modbus_sensor";

float convert_uint32_to_float(uint16_t low, uint16_t high) {
  float fValue = 0.0;
  uint32_t value = ((uint32_t)low << 16) | high;
  memcpy(&fValue, &value, sizeof(value));
  return fValue;
}

void modbus_sensor_test(int mb_sensor) {
  uint8_t value[30] = { 0 };
  int data_len = 0;
  mb_characteristic_info_t mb_characteristic[3] = { 0 };

  // Refer to the modbus function code and register in EC Water RK500-13.pdf
  // << Water EC Sensor >>
  // Slave ID = 0x07, Function Code = 0x03, Start Address = 0x0000, Read Register Len = 0x000A
  // Data type = Binary Data, Data Len = 20 bytes
  // mb_characteristic_info_t mb_characteristic[1] = { { 0, "Data", "Binary", DATA_BINARY, 20, 0x07, MB_HOLDING_REG,
  // 0x0000,
  //                                                    0x000A } };
  // << Soil EC Sensor >>
  // mb_characteristic_info_t mb_characteristic[1] = { { 0, "Data", "Binary", DATA_BINARY, 2, 0x05, MB_HOLDING_REG,
  // 0x0000,
  //                                                    0x0003 } };
  // Or
  // mb_characteristic_info_t mb_characteristic[3] = {
  //  { 0, "Temperature", "C", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0000, 0x0001 },
  //  { 1, "Moisture", "%", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0001, 0x0001 },
  //  { 2, "EC", "mS", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0002, 0x0001 },
  // };

  if (mb_sensor == SOIL_EC_SENSOR) {
    mb_characteristic_info_t mb_soil_ec[3] = {
      { 0, "Temperature", "C", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0000, 0x0001 },
      { 1, "Moisture", "%", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0001, 0x0001 },
      { 2, "EC", "mS", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0002, 0x0001 },
    };
    memcpy(&mb_characteristic, mb_soil_ec, sizeof(mb_soil_ec));
  } else if (mb_sensor == WATER_EC_SENSOR) {
    mb_characteristic_info_t mb_water_ec[3] = {
      { 0, "Data", "Binary", DATA_BINARY, 20, 0x07, MB_HOLDING_REG, 0x0000, 0x000A },
    };
    memcpy(&mb_characteristic, mb_water_ec, sizeof(mb_water_ec));
  }

  mb_set_uart_config(MB_RX_PIN, MB_TX_PIN, RTS_UNCHANGED, CTS_UNCHANGED);
  if (mb_master_init(UART_PORT_NUM, 9600) == -1) {
    LOGE(TAG, "Failed to initialize modbus master");
    return;
  } else {
    LOGI(TAG, "Success to initialize modbus master");
  }

  vTaskDelay(1000 / portTICK_RATE_MS);

  int num_characteristic = (sizeof(mb_characteristic) / sizeof(mb_characteristic[0]));
  mb_master_register_characteristic(&mb_characteristic[0], num_characteristic);

  while (1) {
    for (int i = 0; i < num_characteristic; i++) {
      if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
        LOGE(TAG, "Failed to read value");
      } else {
        for (int i = 0; i < data_len; i++) {
          LOGI(TAG, "value[%d] = [0x%x]", i, value[i]);
        }
        if (mb_sensor == WATER_EC_SENSOR) {
          uint16_t ec_low = 0, ec_high = 0;
          uint16_t temp_low = 0, temp_high = 0;
          uint16_t sal_low = 0, sal_high = 0;
          memcpy(&ec_low, value, 2);
          memcpy(&ec_high, value + 2, 2);
          memcpy(&temp_low, value + 8, 2);
          memcpy(&temp_high, value + 10, 2);
          memcpy(&sal_low, value + 16, 2);
          memcpy(&sal_high, value + 18, 2);
          float ec = convert_uint32_to_float(ec_low, ec_high);
          float sal = convert_uint32_to_float(sal_low, sal_high);
          float temp = convert_uint32_to_float(temp_low, temp_high);
          LOGI(TAG, "ec = %.2f", ec);
          LOGI(TAG, "salinity = %.2f", sal);
          LOGI(TAG, "temperature = %.2f", temp);
        }
        if (mb_sensor == SOIL_EC_SENSOR) {
          if (mb_characteristic[i].cid == 0) {
            uint16_t temp = 0;
            memcpy(&temp, value, 2);
            LOGI(TAG, "temperature = %.2f", (float)(temp / 10.00));
          } else if (mb_characteristic[i].cid == 1) {
            uint16_t mos = 0;
            memcpy(&mos, value, 2);
            LOGI(TAG, "moisture = %.2f", (float)(mos / 10.00));

          } else if (mb_characteristic[i].cid == 2) {
            uint16_t ec = 0;
            memcpy(&ec, value, 2);
            LOGI(TAG, "EC = %.2f", (float)(ec / 1000.00));
          }
        }
      }
      vTaskDelay(500 / portTICK_RATE_MS);
    }
    vTaskDelay(5000 / portTICK_RATE_MS);
  }

  // Or, you can read the modbus register directly using mb_master_send_request() command.
#if 0
  for (int i = 0; i < 10; i++) {
    if (mb_master_send_request(0x05, 0x03, 0x0000, 0x0003, value) == -1) {
      LOGE(TAG, "Failed to send_request");
    } else {
      for (int j = 0; j < 20; j++) {
        LOGI(TAG, "value[%d] = [0x%x]", j, value[j]);
      }
    }
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
#endif

  mb_master_deinit();
}
