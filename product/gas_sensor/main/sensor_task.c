#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "log.h"
#include "sys_status.h"
#include "main.h"
#include "filelog.h"
#include "config.h"
#include "mb_master_rtu.h"
#include "gpio_api.h"
#if (CONFIG_WINSEN_ZE03_FEATURE)
#include "winsen_ze03.h"
#endif
#if (CONFIG_WINSEN_ZCE04B_FEATURE)
#include "winsen_zce04b.h"
#endif
#if (CONFIG_TH01_FEATURE)
#include "th01.h"
#endif

#define OUTPUT 2

typedef enum {
  RENKE_SEC_SLAVE_ID = 0x01,  // RENKE Soil EC sensor
} MB_SLAVE_ADDR;

typedef enum {
  WINSEN_ZE03 = 0x01,
  WINSEN_ZCE04B = 0x02,
  TH01 = 0x03,
} UART_MUX_ADDR;

static const char *TAG = "sensor_task";

int num_characteristic = 1;
mb_characteristic_info_t mb_characteristic[2] = {
  { 0, "RS_ECTH", "Binary", DATA_BINARY, 6, RENKE_SEC_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0003 },
};

float soil_temp = 0.0;
float soil_mos = 0.0;
float soil_ec = 0.0;

void uart_mux_init(void) {
  gpio_init(UART1_MUX_A_GPIO, OUTPUT);
  gpio_init(UART1_MUX_B_GPIO, OUTPUT);
  gpio_write(UART1_MUX_A_GPIO, 0);
  gpio_write(UART1_MUX_B_GPIO, 0);
}

void uart_mux_set(int type) {
  switch (type) {
    case WINSEN_ZE03: {
      gpio_write(UART1_MUX_A_GPIO, 0);
      gpio_write(UART1_MUX_B_GPIO, 0);
    } break;
    case WINSEN_ZCE04B: {
      gpio_write(UART1_MUX_A_GPIO, 1);
      gpio_write(UART1_MUX_B_GPIO, 0);
    } break;
    case TH01: {
      gpio_write(UART1_MUX_A_GPIO, 0);
      gpio_write(UART1_MUX_B_GPIO, 1);
    } break;
    default: break;
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

int sensor_init(void) {
  int res = 0;

  uart_mux_init();
#if (CONFIG_WINSEN_ZE03_FEATURE)
  uart_mux_set(WINSEN_ZE03);
  winsen_ze03_init();
#endif
#if (CONFIG_WINSEN_ZCE04B_FEATURE)
  uart_mux_set(WINSEN_ZCE04B);
  winsen_zce04b_init();
#endif
#if (CONFIG_TH01_FEATURE)
  uart_mux_set(TH01);
  th01_init();
#endif

  mb_set_uart_config(MB_RX_PIN, MB_TX_PIN, RTS_UNCHANGED, CTS_UNCHANGED);

  if (mb_master_init(UART_PORT_NUM, BAUD_RATE) == -1) {
    LOGE(TAG, "Failed to initialize modbus master");
    return res;
  } else {
    LOGI(TAG, "Success to initialize modbus master");
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  mb_master_register_characteristic(&mb_characteristic[0], num_characteristic);

  return res;
}

float convert_bulk_to_saturation_ec(float temp, float mos, float ec) {
  float raw, eb, saturation_ec;

  raw = (mos + 0.6956) / 0.0003879;
  eb = (0.000000002887 * raw * raw * raw) - (0.00002080 * raw * raw) + (0.05276 * raw) - 43.39;
  eb = eb * eb;
  saturation_ec = (80 * mos * ec) / (0.5 * (eb - 4.1));

  return saturation_ec;
}

int read_soil_ec_rs_ecth(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };

  float f_ec = 0.0;

  uint16_t u_temp = 0;
  uint16_t u_mos = 0;
  uint16_t u_ec = 0;

  if (mb_master_read_characteristic(mb_characteristic[0].cid, mb_characteristic[0].name, value, &data_len) == -1) {
    LOGE(TAG, "Failed to read value : %s ", mb_characteristic[0].name);
    res = -1;
  } else {
    memcpy(&u_mos, value, 2);
    memcpy(&u_temp, value + 2, 2);
    memcpy(&u_ec, value + 4, 2);
    soil_temp = (float)(u_temp / 10.00);
    soil_mos = (float)(u_mos / 10.0);
    f_ec = (float)(u_ec / 1000.00);

    soil_ec = convert_bulk_to_saturation_ec(soil_temp, (soil_mos / 100), f_ec);

    LOGI(TAG, "[RS_ECTH] ec : %.2f, moisture : %.2f, temperature : %.2f", soil_ec, soil_mos, soil_temp);
  }

  return res;
}

int sensor_read(void) {
  int rc = 0;

#if (CONFIG_WINSEN_ZE03_FEATURE)
  uart_mux_set(WINSEN_ZE03);
  int gas_nh3 = 0;
  rc = winsen_ze03_read_manual(&gas_nh3);
  if (rc)
    gas_nh3 = 0;
#endif
#if (CONFIG_WINSEN_ZCE04B_FEATURE)
  uart_mux_set(WINSEN_ZCE04B);
  zce04b_data_t gas_data;
  rc = winsen_zce04b_read_manual(&gas_data);
  if (rc) {
    gas_data.co = 0;
    gas_data.h2s = 0;
    gas_data.o2 = 0.0;
    gas_data.ch4 = 0;
  }
#endif
#if (CONFIG_TH01_FEATURE)
  uart_mux_set(TH01);
  th01_data_t th01_data;
  rc = th01_read_manual(&th01_data);
  if (rc) {
    th01_data.temp = 0.0;
    th01_data.mos = 0.0;
  }
#endif

  rc = read_soil_ec_rs_ecth();
  if (rc) {
    soil_temp = 0.0;
    soil_mos = 0.0;
    soil_ec = 0.0;
  }

  LOGI(TAG, "NH3,  CO, H2S,   O2, CH4, S_TEMP, S_MOS, S_EC, A_TEMP, A_MOS");
  LOGI(TAG, "%3d, %3d, %3d, %2.1f, %3d,  %2.2f, %2.2f, %2.2f,  %2.1f, %2.1f", gas_nh3, gas_data.co, gas_data.h2s,
       gas_data.o2, gas_data.ch4, soil_temp, soil_mos, soil_ec, th01_data.temp, th01_data.mos);

  set_log_file_write_flag(1);
  FDATA(DIR_PATH, "%d,%d,%d,%.1f,%d,%.2f,%.2f,%.2f,%.1f,%.1f", gas_nh3, gas_data.co, gas_data.h2s, gas_data.o2,
        gas_data.ch4, soil_temp, soil_mos, soil_ec, th01_data.temp, th01_data.mos);
  set_log_file_write_flag(0);

  return 0;
}
