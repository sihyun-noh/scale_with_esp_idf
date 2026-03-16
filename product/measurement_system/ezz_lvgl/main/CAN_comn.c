
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "CAN_comn.h"  // Include the TWAI port header
#include "board_i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/twai_types.h"

static const char *TAG = "can_task";

static bool driver_installed = false;  // Flag to check if the driver is installed
unsigned long previousMillis = 0;      // Will store last time a message was sent

// TWAI configuration settings
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  // Timing configuration for 500 kbps
// static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
static const twai_filter_config_t f_config =
    TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Filter configuration to accept all messages
static const twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);  // General configuration in normal mode

static inline bool can_use_extended_frame(void) {
  return CAN_FRAME_FORMAT == CAN_FRAME_EXTENDED;
}

static const char *can_frame_format_name(void) {
  return can_use_extended_frame() ? "EXTENDED" : "STANDARD";
}

static esp_err_t validate_can_identifier(uint32_t can_id) {
  if (can_use_extended_frame()) {
    if (can_id > 0x1FFFFFFF) {
      ESP_LOGE(TAG, "Invalid extended CAN ID: 0x%08" PRIx32, can_id);
      return ESP_ERR_INVALID_ARG;
    }
  } else {
    if (can_id > 0x7FF) {
      ESP_LOGE(TAG, "Invalid standard CAN ID: 0x%08" PRIx32, can_id);
      return ESP_ERR_INVALID_ARG;
    }
  }

  return ESP_OK;
}

static bool can_rx_filter_match(const twai_message_t *msg) {
#if !CAN_RX_FILTER_ENABLE
  (void)msg;
  return true;
#else
  if (!msg) {
    return false;
  }

#if CONFIG_APP_RUN_MODE_UPPER
  return msg->extd &&
         (msg->identifier == CAN_RX_FILTER_UPPER_STATUS_ID || msg->identifier == CAN_RX_FILTER_UPPER_STATUS_RPM_ID);
#else
  return (!msg->extd) &&
         (msg->identifier == CAN_RX_FILTER_SIBI_ID_1 || msg->identifier == CAN_RX_FILTER_SIBI_ID_2);
#endif
#endif
}

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
    ESP_LOGI(TAG, "Message is in Extended Format");  // Log message format
  } else {
    ESP_LOGI(TAG, "Message is in Standard Format");  // Log message format
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
/*
uint8_t can_cmd_tabel[11][8] = {
  { 0x38, 0x38, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00 }, { 0x38, 0x38, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00 },
  { 0x38, 0x38, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00 }, { 0x3C, 0x72, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 },
  { 0x3C, 0x72, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00 }
};
*/
uint8_t can_cmd_tabel[4][8] = { { 0x3C, 0x72, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 },
                                { 0x3C, 0x72, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00 } };

// Function to send a message
static esp_err_t send_message(int user_data, uint32_t can_id) {
  esp_err_t id_check = validate_can_identifier(can_id);
  if (id_check != ESP_OK) {
    return id_check;
  }

  // Configure message to transmit
  twai_message_t message = {
    .identifier = can_id,
    .extd = can_use_extended_frame(),
    .rtr = 0,
    .data_length_code = 8,
  };

  memcpy(message.data, can_cmd_tabel[user_data], sizeof(can_cmd_tabel[user_data]));
  print_hex(message.data, sizeof(message.data));

  // Queue message for transmission
  esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(1000));
  if (ret == ESP_OK) {
    printf("Message queued for transmission\n");  // Success message
  } else {
    printf("Failed to queue message for transmission(err : %d)\n", ret);  // Failure message
  }

  return ret;
}

// Function to send a message
static esp_err_t evt_send_message(uint8_t *payload, uint32_t can_id) {
  esp_err_t id_check = validate_can_identifier(can_id);
  if (id_check != ESP_OK) {
    return id_check;
  }

  // Configure message to transmit
  twai_message_t message = {
    .identifier = can_id,
    .extd = can_use_extended_frame(),
    .rtr = 0,
    .data_length_code = 8,
  };

  memcpy(message.data, payload, sizeof(message.data));
  print_hex(message.data, sizeof(message.data));

  // Queue message for transmission
  esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(1000));
  if (ret == ESP_OK) {
    printf("Message queued for transmission\n");  // Success message
  } else {
    printf("Failed to queue message for transmission(err : %d)\n", ret);  // Failure message
  }

  return ret;
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
    ESP_LOGI(TAG, "Driver installed");  // Log driver installation success
  } else {
    ESP_LOGI(TAG, "Failed to install driver");  // Log driver installation failure
    return ESP_FAIL;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    ESP_LOGI(TAG, "Driver started");  // Log driver start success
    ESP_LOGI(TAG, "CAN config: baudrate=500kbps, frame=%s", can_frame_format_name());
  } else {
    ESP_LOGI(TAG, "Failed to start driver");  // Log driver start failure
    return ESP_FAIL;
  }

  // Reconfigure alerts to detect frame receive, Bus-Off error and RX queue full states
  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR |
                              TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS |
                              TWAI_ALERT_TX_FAILED;  // Configure alerts

  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    ESP_LOGI(TAG, "CAN Alerts reconfigured");  // Log alert reconfiguration success
  } else {
    ESP_LOGI(TAG, "Failed to reconfigure alerts");  // Log alert reconfiguration failure
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
    vTaskDelay(pdMS_TO_TICKS(1000));               // Wait before retrying
    ESP_LOGI(TAG, "Failed to deriver installed");  // Log alert reconfiguration failure
    return ESP_FAIL;                               // Return failure
  }

  // Check if alert happened
  uint32_t alerts_triggered;                                            // Variable to hold triggered alerts
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));  // Read triggered alerts
  twai_status_info_t twaistatus;                                        // Variable to hold TWAI status information
  twai_get_status_info(&twaistatus);                                    // Get TWAI status information

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {                         // Check for error passive alert
    ESP_LOGI(TAG, "Alert: TWAI controller has become error passive.");  // Log error passive alert
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {  // Check for bus error alert
    ESP_LOGI(TAG,
             "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");  // Log bus error alert
    ESP_LOGI(TAG, "Bus error count: %" PRIu32, twaistatus.bus_error_count);            // Log bus error count
  }

  if (alerts_triggered & TWAI_ALERT_RX_QUEUE_FULL) {  // Check for RX queue full alert
    ESP_LOGI(TAG,
             "Alert: The RX queue is full causing a received frame to be lost.");  // Log RX queue full alert
    ESP_LOGI(TAG, "RX buffered: %" PRIu32, twaistatus.msgs_to_rx);                 // Log buffered RX messages
    ESP_LOGI(TAG, "RX missed: %" PRIu32, twaistatus.rx_missed_count);              // Log missed RX messages
    ESP_LOGI(TAG, "RX overrun %" PRIu32, twaistatus.rx_overrun_count);             // Log RX overrun count
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
#else
  esp_err_t err = ESP_FAIL;
  if (alerts_triggered & TWAI_ALERT_RX_DATA) {  // If RX data alert is triggered
    while ((err = twai_receive(msg, portMAX_DELAY)) == ESP_OK) {
      if (can_rx_filter_match(msg)) {
        return ESP_OK;
      }
    }
  }
  return err;
#endif
}

// Function to transmit messages
esp_err_t waveshare_twai_transmit(int user_data, uint32_t can_id) {
  if (!driver_installed) {
    // Driver not installed
    vTaskDelay(pdMS_TO_TICKS(1000));               // Wait before retrying
    ESP_LOGI(TAG, "Failed to deriver installed");  // Log alert reconfiguration failure
    return ESP_FAIL;                               // Return failure status
  }
  // Check if alert happened
  uint32_t alerts_triggered;                                            // Variable to store triggered alerts
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));  // Read alerts
  twai_status_info_t twaistatus;                                        // Variable to store TWAI status information
  twai_get_status_info(&twaistatus);                                    // Get TWAI status information

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    ESP_LOGI(TAG, "Alert: TWAI controller has become error passive.");  // Log error passive alert
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    ESP_LOGI(TAG,
             "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");  // Log bus error alert
    ESP_LOGI(TAG, "Bus error count: %" PRIu32, twaistatus.bus_error_count);            // Log bus error count
  }
  if (alerts_triggered & TWAI_ALERT_TX_FAILED) {
    ESP_LOGI(TAG, "Alert: The Transmission failed.");                  // Log transmission failure alert
    ESP_LOGI(TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);     // Log buffered messages count
    ESP_LOGI(TAG, "TX error: %" PRIu32, twaistatus.tx_error_counter);  // Log transmission error count
    ESP_LOGI(TAG, "TX failed: %" PRIu32, twaistatus.tx_failed_count);  // Log failed transmission count
  }
  if (alerts_triggered & TWAI_ALERT_TX_SUCCESS) {
    ESP_LOGI(TAG, "Alert: The Transmission was successful.");       // Log transmission success alert
    ESP_LOGI(TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);  // Log buffered messages count
  }
#if 0
  // Send message
  unsigned long currentMillis = esp_timer_get_time() / 1000;  // Get current time in milliseconds
  if (currentMillis - previousMillis >= TRANSMIT_RATE_MS) {   // Check if it's time to send the message
    previousMillis = currentMillis;                           // Update last send time
    send_message(user_data);                                  // Call send message function
  }
#endif

  return send_message(user_data, can_id);  // Call send message function
}

// Function to transmit messages
esp_err_t evt_twai_transmit(uint8_t *payload, uint32_t can_id) {
  if (!driver_installed) {
    // Driver not installed
    vTaskDelay(pdMS_TO_TICKS(1000));               // Wait before retrying
    ESP_LOGI(TAG, "Failed to deriver installed");  // Log alert reconfiguration failure
    return ESP_FAIL;                               // Return failure status
  }
  // Check if alert happened
  uint32_t alerts_triggered;                                            // Variable to store triggered alerts
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));  // Read alerts
  twai_status_info_t twaistatus;                                        // Variable to store TWAI status information
  twai_get_status_info(&twaistatus);                                    // Get TWAI status information

  // Handle alerts
  if (alerts_triggered & TWAI_ALERT_ERR_PASS) {
    ESP_LOGI(TAG, "Alert: TWAI controller has become error passive.");  // Log error passive alert
  }
  if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    ESP_LOGI(TAG,
             "Alert: A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus.");  // Log bus error alert
    ESP_LOGI(TAG, "Bus error count: %" PRIu32, twaistatus.bus_error_count);            // Log bus error count
  }
  if (alerts_triggered & TWAI_ALERT_TX_FAILED) {
    ESP_LOGI(TAG, "Alert: The Transmission failed.");                  // Log transmission failure alert
    ESP_LOGI(TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);     // Log buffered messages count
    ESP_LOGI(TAG, "TX error: %" PRIu32, twaistatus.tx_error_counter);  // Log transmission error count
    ESP_LOGI(TAG, "TX failed: %" PRIu32, twaistatus.tx_failed_count);  // Log failed transmission count
  }
  if (alerts_triggered & TWAI_ALERT_TX_SUCCESS) {
    ESP_LOGI(TAG, "Alert: The Transmission was successful.");       // Log transmission success alert
    ESP_LOGI(TAG, "TX buffered: %" PRIu32, twaistatus.msgs_to_tx);  // Log buffered messages count
  }
#if 0
  // Send message
  unsigned long currentMillis = esp_timer_get_time() / 1000;  // Get current time in milliseconds
  if (currentMillis - previousMillis >= TRANSMIT_RATE_MS) {   // Check if it's time to send the message
    previousMillis = currentMillis;                           // Update last send time
    send_message(user_data);                                  // Call send message function
  }
#endif

  return evt_send_message(payload, can_id);  // Call send message function
}
