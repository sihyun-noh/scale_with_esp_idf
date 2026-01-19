
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "CAN_comn.h"  // Include the TWAI port header
#include "board_i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/twai_types.h"

#include "ui/ui.h"

static const char *TAG = "can_task";

static bool driver_installed = false;  // Flag to check if the driver is installed
unsigned long previousMillis = 0;      // Will store last time a message was sent

// TWAI configuration settings
// static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  // Timing configuration for 500 kbps
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
static const twai_filter_config_t f_config =
    TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Filter configuration to accept all messages
static const twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);  // General configuration in normal mode

#if 0
static esp_err_t i2c_master_init(void) {
  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(i2c_master_port, &conf);

  return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
#endif

uint8_t user_data_ID;
uint32_t user_data_value;

static void handle_rx_message(twai_message_t message) {  // Handle received TWAI message
  // Process received message
  if (message.extd) {
    ESP_LOGI(EXAMPLE_TAG, "Message is in Extended Format");  // Log message format
  } else {
    ESP_LOGI(EXAMPLE_TAG, "Message is in Standard Format");  // Log message format
  }
  printf("ID: %lx\nByte:", message.identifier);           // Print message identifier
  if (!(message.rtr)) {                                   // Check if it's not a Remote Transmission Request (RTR)
    for (int i = 0; i < message.data_length_code; i++) {  // Loop through message data
      printf(" %d = %02x,", i, message.data[i]);          // Print each byte in hex format
    }
    printf("\r\n");  // New line after printing data
  }

  if (message.data[1] == 0x38) {
    user_data_ID = message.data[3];

    user_data_value = (uint32_t)message.data[4] | (uint32_t)message.data[5] << 8 | (uint32_t)message.data[6] << 16 |
                      (uint32_t)message.data[7] << 24;

    switch (user_data_ID) {
      case 0x01: lv_label_set_text_fmt(ui_Label_10, "U1:%d", (int)user_data_value); break;
      case 0x02: lv_label_set_text_fmt(ui_Label_11, "U2:%d", (int)user_data_value); break;
      case 0x03: lv_label_set_text_fmt(ui_Label_12, "U3:%d", (int)user_data_value); break;
      case 0x04: lv_label_set_text_fmt(ui_Label_13, "U4:%d", (int)user_data_value); break;
      case 0x05: lv_label_set_text_fmt(ui_Label_14, "U5:%d", (int)user_data_value); break;
      case 0x06: lv_label_set_text_fmt(ui_Label_15, "U6:%d", (int)user_data_value); break;
      case 0x07: lv_label_set_text_fmt(ui_Label_16, "U7:%d", (int)user_data_value); break;
      case 0x08: lv_label_set_text_fmt(ui_Label_17, "U8:%d", (int)user_data_value); break;
      case 0x09: lv_label_set_text_fmt(ui_Label_18, "U9:%d", (int)user_data_value); break;

      default: break;
    }
  }
  // read motor voltage
  if (message.data[1] == 0x72) {
    user_data_ID = message.data[3];
    user_data_value = (uint32_t)message.data[4] | (uint32_t)message.data[5] << 8 | (uint32_t)message.data[6] << 16 |
                      (uint32_t)message.data[7] << 24;
    float v_val;
    memcpy(&v_val, &user_data_value, sizeof(v_val));

    char buf[32] = { 0 };
    snprintf(buf, sizeof(buf), "%+.2f", v_val);  // 예: +12.00

    switch (user_data_ID) {
      case 0x01: lv_label_set_text(ui_Label_volt_RC1_M1, buf); break;
      case 0x02: lv_label_set_text(ui_Label_volt_RC1_M2, buf); break;

      default: break;
    }
  }
}

void print_hex(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02X%s", buf[i], (i + 1 < len) ? " " : "");
  }
  printf("\n");
}

uint8_t can_cmd_tabel[11][8] = {
  { 0x38, 0x38, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00 }, { 0x3C, 0x72, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 },
  { 0x3C, 0x72, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00 }
};

// Function to send a message
static void send_message(int user_data) {
  // Configure message to transmit
  twai_message_t message;        // Message structure
  message.identifier = 0x01;     // Message identifier
  message.data_length_code = 8;  // Data length code

  memcpy(message.data, can_cmd_tabel[user_data], sizeof(can_cmd_tabel[user_data]));
  print_hex(message.data, sizeof(message.data));

  // Queue message for transmission
  esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(1000));
  if (ret == ESP_OK) {
    printf("Message queued for transmission\n");  // Success message
  } else {
    printf("Failed to queue message for transmission(err : %d)\n", ret);  // Failure message
  }
}

// Function to send a message
static void evt_send_message(uint8_t *payload) {
  // Configure message to transmit
  twai_message_t message;        // Message structure
  message.identifier = 0x01;     // Message identifier
  message.data_length_code = 8;  // Data length code

  memcpy(message.data, payload, sizeof(message.data));
  print_hex(message.data, sizeof(message.data));

  // Queue message for transmission
  esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(1000));
  if (ret == ESP_OK) {
    printf("Message queued for transmission\n");  // Success message
  } else {
    printf("Failed to queue message for transmission(err : %d)\n", ret);  // Failure message
  }
}

esp_err_t waveshare_twai_init()  // Initialize TWAI driver
{
  ESP_ERROR_CHECK(board_i2c_init());

// When USB_SEL is HIGH, it enables FSUSB42UMX chip and gpio19,gpio20 wired CAN_TX CAN_RX, and then dont use USB
// Function
#if 0
  uint8_t write_buf = 0x01;
  i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  write_buf = 0x20;
  i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
#endif
  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    ESP_LOGI(EXAMPLE_TAG, "Driver installed");  // Log driver installation success
  } else {
    ESP_LOGI(EXAMPLE_TAG, "Failed to install driver");  // Log driver installation failure
    return ESP_FAIL;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    ESP_LOGI(EXAMPLE_TAG, "Driver started");  // Log driver start success
  } else {
    ESP_LOGI(EXAMPLE_TAG, "Failed to start driver");  // Log driver start failure
    return ESP_FAIL;
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR |
                              TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS |
                              TWAI_ALERT_TX_FAILED;  // Configure alerts

  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    ESP_LOGI(EXAMPLE_TAG, "CAN Alerts reconfigured");  // Log alert reconfiguration success
  } else {
    ESP_LOGI(EXAMPLE_TAG, "Failed to reconfigure alerts");  // Log alert reconfiguration failure
    return ESP_FAIL;
  }

  // TWAI driver is now successfully installed and started
  driver_installed = true;  // Set driver installed flag
  return ESP_OK;            // Return success
}

esp_err_t waveshare_twai_receive(twai_message_t *msg)  // Receive messages via TWAI
{
  if (!driver_installed) {  // Check if the driver is installed
    // Driver not installed
    vTaskDelay(pdMS_TO_TICKS(1000));                       // Wait before retrying
    ESP_LOGI(EXAMPLE_TAG, "Failed to deriver installed");  // Log alert reconfiguration failure
    return ESP_FAIL;                                       // Return failure
  }

  // Check if alert happened
  uint32_t alerts_triggered;                                            // Variable to hold triggered alerts
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));  // Read triggered alerts
  twai_status_info_t twaistatus;                                        // Variable to hold TWAI status information
  twai_get_status_info(&twaistatus);                                    // Get TWAI status information

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {                                 // Check for error passive alert
    ESP_LOGI(EXAMPLE_TAG, "Alert: TWAI controller has become error passive.");  // Log error passive alert
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {  // Check for bus error alert
    ESP_LOGI(EXAMPLE_TAG,
             "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");  // Log bus error alert
    ESP_LOGI(EXAMPLE_TAG, "Bus error count: %" PRIu32, twaistatus.bus_error_count);    // Log bus error count
  }

  if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {  // Check for RX queue full alert
    ESP_LOGI(EXAMPLE_TAG,
             "Alert: The RX queue is full causing a received frame to be lost.");  // Log RX queue full alert
    ESP_LOGI(EXAMPLE_TAG, "RX buffered: %" PRIu32, twaistatus.msgs_to_rx);         // Log buffered RX messages
    ESP_LOGI(EXAMPLE_TAG, "RX missed: %" PRIu32, twaistatus.rx_missed_count);      // Log missed RX messages
    ESP_LOGI(EXAMPLE_TAG, "RX overrun %" PRIu32, twaistatus.rx_overrun_count);     // Log RX overrun count
  }
#if 0
  // Check if message is received
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {  // If RX data alert is triggered
    // One or more messages received. Handle all.
    twai_message_t message;                        // Variable to hold received message
    while (twai_receive(&message, 0) == ESP_OK) {  // Receive messages until none are left
      handle_rx_message(message);                  // Process each received message
    }
  }
  return ESP_OK;  // Return success
#endif
  esp_err_t err = ESP_FAIL;
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {  // If RX data alert is triggered
    err = twai_receive(msg, portMAX_DELAY);     // 실시간 대기
  }
  return err;
}

// Function to transmit messages
esp_err_t waveshare_twai_transmit(int user_data) {
  if (!driver_installed) {
    // Driver not installed
    vTaskDelay(pdMS_TO_TICKS(1000));                       // Wait before retrying
    ESP_LOGI(EXAMPLE_TAG, "Failed to deriver installed");  // Log alert reconfiguration failure
    return ESP_FAIL;                                       // Return failure status
  }
  // Check if alert happened
  uint32_t alerts_triggered;                                            // Variable to store triggered alerts
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));  // Read alerts
  twai_status_info_t twaistatus;                                        // Variable to store TWAI status information
  twai_get_status_info(&twaistatus);                                    // Get TWAI status information

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    ESP_LOGI(EXAMPLE_TAG, "Alert: TWAI controller has become error passive.");  // Log error passive alert
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    ESP_LOGI(EXAMPLE_TAG,
             "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");  // Log bus error alert
    ESP_LOGI(EXAMPLE_TAG, "Bus error count: %" PRIu32, twaistatus.bus_error_count);    // Log bus error count
  }
  if (alerts_triggered & TWAI_ALERT_TX_FAILED) {
    ESP_LOGI(EXAMPLE_TAG, "Alert: The Transmission failed.");                  // Log transmission failure alert
    ESP_LOGI(EXAMPLE_TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);     // Log buffered messages count
    ESP_LOGI(EXAMPLE_TAG, "TX error: %" PRIu32, twaistatus.tx_error_counter);  // Log transmission error count
    ESP_LOGI(EXAMPLE_TAG, "TX failed: %" PRIu32, twaistatus.tx_failed_count);  // Log failed transmission count
  }
  if (alerts_triggered & TWAI_ALERT_TX_SUCCESS) {
    ESP_LOGI(EXAMPLE_TAG, "Alert: The Transmission was successful.");       // Log transmission success alert
    ESP_LOGI(EXAMPLE_TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);  // Log buffered messages count
  }
#if 0
  // Send message
  unsigned long currentMillis = esp_timer_get_time() / 1000;  // Get current time in milliseconds
  if (currentMillis - previousMillis >= TRANSMIT_RATE_MS) {   // Check if it's time to send the message
    previousMillis = currentMillis;                           // Update last send time
    send_message(user_data);                                  // Call send message function
  }
#endif

  send_message(user_data);  // Call send message function
  return ESP_OK;            // Return success status
}

// Function to transmit messages
esp_err_t evt_twai_transmit(uint8_t *payload) {
  if (!driver_installed) {
    // Driver not installed
    vTaskDelay(pdMS_TO_TICKS(1000));                       // Wait before retrying
    ESP_LOGI(EXAMPLE_TAG, "Failed to deriver installed");  // Log alert reconfiguration failure
    return ESP_FAIL;                                       // Return failure status
  }
  // Check if alert happened
  uint32_t alerts_triggered;                                            // Variable to store triggered alerts
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));  // Read alerts
  twai_status_info_t twaistatus;                                        // Variable to store TWAI status information
  twai_get_status_info(&twaistatus);                                    // Get TWAI status information

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    ESP_LOGI(EXAMPLE_TAG, "Alert: TWAI controller has become error passive.");  // Log error passive alert
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    ESP_LOGI(EXAMPLE_TAG,
             "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");  // Log bus error alert
    ESP_LOGI(EXAMPLE_TAG, "Bus error count: %" PRIu32, twaistatus.bus_error_count);    // Log bus error count
  }
  if (alerts_triggered & TWAI_ALERT_TX_FAILED) {
    ESP_LOGI(EXAMPLE_TAG, "Alert: The Transmission failed.");                  // Log transmission failure alert
    ESP_LOGI(EXAMPLE_TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);     // Log buffered messages count
    ESP_LOGI(EXAMPLE_TAG, "TX error: %" PRIu32, twaistatus.tx_error_counter);  // Log transmission error count
    ESP_LOGI(EXAMPLE_TAG, "TX failed: %" PRIu32, twaistatus.tx_failed_count);  // Log failed transmission count
  }
  if (alerts_triggered & TWAI_ALERT_TX_SUCCESS) {
    ESP_LOGI(EXAMPLE_TAG, "Alert: The Transmission was successful.");       // Log transmission success alert
    ESP_LOGI(EXAMPLE_TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);  // Log buffered messages count
  }
#if 0
  // Send message
  unsigned long currentMillis = esp_timer_get_time() / 1000;  // Get current time in milliseconds
  if (currentMillis - previousMillis >= TRANSMIT_RATE_MS) {   // Check if it's time to send the message
    previousMillis = currentMillis;                           // Update last send time
    send_message(user_data);                                  // Call send message function
  }
#endif

  evt_send_message(payload);  // Call send message function
  return ESP_OK;              // Return success status
}
