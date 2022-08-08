#include "syslog.h"

#include "mb_master_impl.h"
#include "mbcontroller.h"

#include <stdio.h>
#include <string.h>

#define MAX_CID_PARAMS 5

static const char *TAG = "mb_master";

static mb_parameter_descriptor_t cid_parameters[MAX_CID_PARAMS];

static mb_param_type_t s_register_types[4] = { MB_PARAM_HOLDING, MB_PARAM_INPUT, MB_PARAM_COIL, MB_PARAM_DISCRETE };

static mb_param_type_t get_register_type(int type) {
  return s_register_types[type];
}

int mb_master_init_impl(int port, int baudrate, uart_params_t *uart) {
  // Pointer to allocate interface structure
  void *master_handler = NULL;
  // Initialize and start Modbus master for serial port
  mb_communication_info_t comm = { .port = port, .mode = MB_MODE_RTU, .baudrate = baudrate, .parity = MB_PARITY_NONE };

  esp_err_t ret = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);

  if (master_handler == NULL || ret != ESP_OK) {
    LOGE(TAG, "modbus master initialization fail.");
    return -1;
  }

  if (ESP_OK != mbc_master_setup((void *)&comm)) {
    LOGE(TAG, "modbus master setup fail.");
    return -1;
  }

  // Set UART pin numbers
  if (ESP_OK != uart_set_pin(port, uart->tx, uart->rx, uart->rts, uart->cts)) {
    LOGE(TAG, "modbus master uart set pin fail.");
    return -1;
  }

  // uart_driver_install should be called in mbc_master_start(). so it was called before calling uart apis.
  ret = mbc_master_start();

  LOGI(TAG, "rx = %d, tx = %d", uart->rx, uart->tx);
  LOGI(TAG, "port = %d, speed = %d", comm.port, comm.baudrate);
  LOGI(TAG, "mode = %d, parity = %d", comm.mode, comm.parity);

  if (ESP_OK != uart_set_mode(port, UART_MODE_RS485_HALF_DUPLEX)) {
    LOGE(TAG, "modbus master uart set mode fail.");
    return -1;
  }

  return (ret == ESP_OK) ? 0 : -1;
}

int mb_master_register_characteristic_impl(mb_characteristic_info_t *mb_characteristic, int num_characteristic) {
  if (num_characteristic > MAX_CID_PARAMS) {
    LOGE(TAG, "Exceed to the maximum characteristic count");
    return -1;
  }

  for (int i = 0; i < num_characteristic; i++) {
    cid_parameters[i].cid = mb_characteristic[i].cid;
    cid_parameters[i].param_key = mb_characteristic[i].name;
    cid_parameters[i].param_units = mb_characteristic[i].units;
    cid_parameters[i].mb_slave_addr = mb_characteristic[i].slave_id;
    cid_parameters[i].mb_param_type = get_register_type(mb_characteristic[i].register_type);
    cid_parameters[i].mb_reg_start = mb_characteristic[i].reg_addr;
    cid_parameters[i].mb_size = mb_characteristic[i].read_len;
    cid_parameters[i].param_offset = 0;
    cid_parameters[i].param_type = mb_characteristic[i].data_type;
    cid_parameters[i].param_size = mb_characteristic[i].data_len;
    cid_parameters[i].param_opts.opt1 = 0;
    cid_parameters[i].param_opts.opt2 = 0;
    cid_parameters[i].param_opts.opt3 = 0;
    cid_parameters[i].access = PAR_PERMS_READ_WRITE_TRIGGER;
  }
  return mbc_master_set_descriptor(&cid_parameters[0], num_characteristic);
}

int mb_master_read_characteristic_impl(uint16_t cid, char *name, uint8_t *data, int *data_len) {
  const mb_parameter_descriptor_t *param_descriptor = NULL;
  uint8_t type = 0;

  // Get data from parameters description table
  esp_err_t ret = mbc_master_get_cid_info(cid, &param_descriptor);
  if ((ret != ESP_ERR_NOT_FOUND) && (param_descriptor != NULL)) {
    if ((cid == param_descriptor->cid) && (strcmp(name, param_descriptor->param_key) == 0)) {
      ret =
          mbc_master_get_parameter(param_descriptor->cid, (char *)param_descriptor->param_key, (uint8_t *)data, &type);
      if (ret == ESP_OK) {
        LOGI(TAG, "Characteristic #%d %s len = (%d) value = (0x%08x) read successful.", param_descriptor->cid,
             (char *)param_descriptor->param_key, param_descriptor->param_size, *(uint32_t *)data);
        *data_len = param_descriptor->param_size;
        return 0;
      }
    }
  }
  LOGE(TAG, "Failed to read the value of cid = %d, name = %s", cid, name);
  return -1;
}

int mb_master_write_characteristic_impl(uint16_t cid, char *name, uint8_t *data) {
  uint8_t type = 0;

  esp_err_t ret = mbc_master_set_parameter(cid, name, (uint8_t *)data, &type);
  if (ret == ESP_OK) {
    LOGI(TAG, "Write parameter data successfully.");
  } else {
    LOGE(TAG, "Failed to write data, err = 0x%x (%s).", (int)ret, (char *)esp_err_to_name(ret));
  }
  return (ret == ESP_OK) ? 0 : -1;
}

int mb_master_send_request_impl(uint8_t slave_id, uint8_t command, uint16_t reg_start, uint16_t size, void *data) {
  mb_param_request_t mb_master_req;

  mb_master_req.slave_addr = slave_id;
  mb_master_req.command = command;
  mb_master_req.reg_start = reg_start;
  mb_master_req.reg_size = size;

  esp_err_t ret = ESP_OK;

  ret = mbc_master_send_request(&mb_master_req, data);
  return (ret == ESP_OK) ? 0 : -1;
}

void mb_master_deinit_impl(void) {
  mbc_master_destroy();
}
