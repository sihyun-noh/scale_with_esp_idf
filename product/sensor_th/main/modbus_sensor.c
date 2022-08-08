// Olimex Board PIN numbers for U1RX and U1TX
#define MB_RX_PIN 36
#define MB_TX_PIN 4

#define RTS_UNCHANGED (-1)
#define CTS_UNCHANGED (-1)

#define UART_PORT_NUM 1

void modbus_sensor_test(void) {
  uint8_t value[30] = { 0 };
  int data_len = 0;

  // Refer to the modbus function code and register in EC Water RK500-13.pdf
  // Slave ID = 0x07, Function Code = 0x03, Start Address = 0x0000, Read Register Len = 0x000A
  // Data type = Binary Data, Data Len = 20 bytes
  mb_characteristic_info_t mb_characteristic[1] = { { 0, "Data", "Binary", DATA_BINARY, 20, 0x07, MB_HOLDING_REG,
                                                      0x0000, 0x000A } };

  mb_set_uart_config(MB_RX_PIN, MB_TX_PIN, RTS_UNCHANGED, CTS_UNCHANGED);
  if (mb_master_init(UART_PORT_NUM, 9600) == -1) {
    LOGE(TAG, "Failed to initialize modbus master");
    return;
  } else {
    LOGI(TAG, "Success to initialize modbus master");
  }

  vTaskDelay(1000 / portTICK_RATE_MS);

  mb_master_register_characteristic(&mb_characteristic[0], 1);

  while (1) {
    if (mb_master_read_characteristic(0, "Data", value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
    } else {
      for (int i = 0; i < data_len; i++) {
        LOGI(TAG, "value[%d] = [0x%x]", i, value[i]);
      }
    }
    vTaskDelay(5000 / portTICK_RATE_MS);
  }

  // Or, you can read the modbus register directly using mb_master_send_request() command.
#if 0
  for (int i = 0; i < 10; i++) {
    if (mb_master_send_request(0x07, 0x03, 0x0000, 0x000A, value) == -1) {
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
