#include "easy_setup.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "shell_console.h"
#include "syscfg.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"
#include "wifi_manager.h"
#include "esp_sleep.h"
#include "event_ids.h"
#include "time_api.h"
#include "monitoring.h"
#include "sysfile.h"
#include "config.h"
#include "filelog.h"
#include "main.h"

#include <string.h>

const char* TAG = "main_app";

sc_ctx_t* ctx = NULL;

static bool b_sensor_pub;

extern void modbus_sensor_test(int mb_sensor);

extern int sensor_init(void);
#if (SENSOR_TYPE == SHT3X)
extern int read_temperature_humidity(int op_mode, char* temperature, char* humidity);
#elif (SENSOR_TYPE == SCD4X)
extern int read_co2_temperature_humidity(char* co2, char* temperature, char* humidity);
#elif (SENSOR_TYPE == RK520_02)
extern int read_soil_ec(char *temperature, char *moisture, char *ec);
#elif (SENSOR_TYPE == SWSR7500)
extern int read_solar_radiation(char* pyranometer);
#endif
extern int read_battery_percentage(void);
extern int sensor_comparison(float m_sensor, float sensor, const float const_comparison);

extern int start_mqttc(void);
extern void stop_mqttc(void);
extern void mqtt_publish_sensor_data(void);

#if defined(CONFIG_LED_FEATURE)
extern void create_led_task(void);
#endif

extern int start_file_server(uint32_t port);

static void generate_default_syscfg(void);

RTC_DATA_ATTR float memory_sensor_buf;
RTC_DATA_ATTR uint8_t m_alive_count;
RTC_DATA_ATTR uint8_t m_timezone_set;

static operation_mode_t s_curr_mode;

#define SENSOR_TEST 0

void set_operation_mode(operation_mode_t mode) {
  s_curr_mode = mode;
}

operation_mode_t get_operation_mode(void) {
  if (is_fwupdate()) {
    s_curr_mode = OTA_FWUPDATE_MODE;
  }
  return s_curr_mode;
}

/**
 * @brief Handler function to stop interactive command shell
 *
 * @note This function is called in shell_command_impl.c
 */
void stop_shell(void) {
  if (ctx) {
    sc_stop(ctx);
  }
}

/* The code below is only used for testing purpose until manufacturing data is applied */
static void generate_default_syscfg(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
  if (model_name[0] == 0) {
#if (SENSOR_TYPE == SHT3X)
    syscfg_set(MFG_DATA, "model_name", "GLSTH");
#elif (SENSOR_TYPE == SCD4X)
    syscfg_set(MFG_DATA, "model_name", "GLSCO");
#elif (SENSOR_TYPE == RK520_02)
    syscfg_set(MFG_DATA, "model_name", "GLSSE");
#elif (SENSOR_TYPE == SWSR7500)
    syscfg_set(MFG_DATA, "model_name", "GLSSO");
#endif
  }

  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
  if (power_mode[0] == 0) {
    syscfg_set(MFG_DATA, "power_mode", "B");
  }

  syscfg_set(CFG_DATA, "fw_version", FW_VERSION);
}

int system_init(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  if (ret != ESP_OK) {
    return ERR_NVS_FLASH;
  }

  ret = wifi_user_init();
  if (ret != 0) {
    return ERR_WIFI_INIT;
  }

  ret = syscfg_init();
  if (ret != 0) {
    return ERR_SYSCFG_INIT;
  }

  ret = syscfg_open();
  if (ret != 0) {
    return ERR_SYSCFG_OPEN;
  }

  ret = sys_stat_init();
  if (ret != 0) {
    return ERR_SYS_STATUS_INIT;
  }

  ret = monitoring_init();
  if (ret != 0) {
    return ERR_MONITORING_INIT;
  }

  ret = sysevent_create();
  if (ret != 0) {
    return ERR_SYSEVENT_CREATE;
  }

  syslog_init();

  ret = init_sysfile();
  if (ret != 0) {
    return ERR_SPIFFS_INIT;
  }

  // Generate the default manufacturing data if there is no data in mfg partition.
  generate_default_syscfg();

  return SYSINIT_OK;
}

int alive_check_task() {
  int ret = 0;
  LOGI(TAG, "%d", m_alive_count);
  if (m_alive_count >= 20) {
    m_alive_count = 0;
    ret = 0;
  } else {
    m_alive_count = m_alive_count + 1;
    ret = 1;
  }
  return ret;
}

int sleep_timer_wakeup(int wakeup_time_sec) {
  int ret = 0;
  LOGI(TAG, "Enabling timer wakeup, %ds\n", wakeup_time_sec);
  ret = esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
  esp_deep_sleep_start();
  return ret;
}

void battery_loop_task(void) {
#if (SENSOR_TEST == 1)
  float random = 0;
  char buf[20] = { 0 };
#else
  int rc = 0;
#if (SENSOR_TYPE == SHT3X)
  float temperature = 0;
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
#elif (SENSOR_TYPE == SCD4X)
  float co2 = 0;
  char s_co2[20] = { 0 };
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
#elif (SENSOR_TYPE == RK520_02)
  float temperature = 0.0;
  float moisture = 0.0;
  float ec = 0.0;
  char s_temperature[20] = { 0 };
  char s_moisture[20] = { 0 };
  char s_ec[20] = { 0 };
#elif (SENSOR_TYPE == SWSR7500)
  float pyranometer = 0.0;
  char s_pyranometer[20] = { 0 };
#endif
#endif

  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case SENSOR_INIT_MODE: {
        LOGI(TAG, "SENSOR_INIT_MODE");
#if (SENSOR_TEST == 1)
        set_operation_mode(SENSOR_READ_MODE);
#else
        if ((rc = sensor_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize sensor");
          set_operation_mode(SLEEP_MODE);
        } else {
          set_operation_mode(SENSOR_READ_MODE);
        }
#endif
      } break;
      case SENSOR_READ_MODE: {
        LOGI(TAG, "SENSOR_READ_MODE");
        b_sensor_pub = false;
#if (SENSOR_TEST == 1)
        random = (float)(rand() % 10000) / 100;  // 난수 생성
        snprintf(buf, sizeof(buf), "%f", random);
        sysevent_set(I2C_TEMPERATURE_EVENT, buf);
        sysevent_set(I2C_HUMIDITY_EVENT, buf);
        sysevent_set(ADC_BATTERY_EVENT, buf);
#if (SENSOR_TYPE == SCD4X)
        sysevent_set(I2C_CO2_EVENT, buf);
#endif
        set_operation_mode(EASY_SETUP_MODE);
#else

#if (SENSOR_TYPE == SHT3X)
        if ((rc = read_temperature_humidity(BATTERY_OP_MODE, s_temperature, s_humidity)) == CHECK_OK) {
          temperature = atof(s_temperature);
          if ((rc = alive_check_task()) == CHECK_OK) {
            b_sensor_pub = true;
          } else if ((rc = sensor_comparison(memory_sensor_buf, temperature, 0.5)) == SENSOR_PUB) {
            b_sensor_pub = true;
          }
          if (b_sensor_pub) {
            memory_sensor_buf = temperature;
            FLOGI(TAG, "temperature : %.2f, m_temperature : %.2f", temperature, memory_sensor_buf);
            sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
#elif (SENSOR_TYPE == SCD4X)
        if ((rc = read_co2_temperature_humidity(s_co2, s_temperature, s_humidity)) == CHECK_OK) {
          co2 = atof(s_co2);
          if ((rc = alive_check_task()) == CHECK_OK) {
            b_sensor_pub = true;
          } else if ((rc = sensor_comparison(memory_sensor_buf, co2, 40)) == SENSOR_PUB) {
            b_sensor_pub = true;
          }
          if (b_sensor_pub) {
            memory_sensor_buf = co2;
            FLOGI(TAG, "co2 : %.2f, m_co2 : %.2f", co2, memory_sensor_buf);
            sysevent_set(I2C_CO2_EVENT, s_co2);
            sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
#elif (SENSOR_TYPE == RK520_02)
        if ((rc = read_soil_ec(s_temperature, s_moisture, s_ec)) == CHECK_OK) {
          ec = atof(s_ec);
          temperature = atof(s_temperature);
          moisture = atof(s_moisture);
          LOGI(TAG, "ec : %.2f, moisture : %.2f, temperature : %.2f", ec, moisture, temperature);
          FLOGI(TAG, "ec : %.2f, moisture : %.2f, temperature : %.2f", ec, moisture, temperature);
          b_sensor_pub = true;
          if (b_sensor_pub) {
            sysevent_set(MB_EC_EVENT, s_ec);
            sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(MB_MOISTURE_EVENT, s_moisture);
#elif (SENSOR_TYPE == SWSR7500)
        if ((rc = read_solar_radiation(s_pyranometer)) == CHECK_OK) {
          pyranometer = atof(s_pyranometer);
          LOGI(TAG, "pyranometer : %.2f", pyranometer);
          FLOGI(TAG, "pyranometer : %.2f", pyranometer);
          b_sensor_pub = true;
          if (b_sensor_pub) {
            sysevent_set(MB_PYRANOMETER_EVENT, s_pyranometer);
#endif
            if ((rc = read_battery_percentage()) != CHECK_OK) {
              // Failed to read battery percentage, it will be set 0
              sysevent_set(ADC_BATTERY_EVENT, "0");
            }
            set_operation_mode(EASY_SETUP_MODE);
          } else {
            set_operation_mode(SLEEP_MODE);
          }
        } else {
          rc = ERR_SENSOR_READ;
          LOGE(TAG, "sensor read, error = [%d]", rc);
          FLOGI(TAG, "sensor read, error = [%d]", rc);
          set_operation_mode(SLEEP_MODE);
        }
#endif
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_easy_setup_task();
        set_operation_mode(TIME_ZONE_SET_MODE);
      } break;
      case TIME_ZONE_SET_MODE: {
        if (is_device_configured() && is_device_onboard()) {
          if (!m_timezone_set) {
            if (tm_is_timezone_set() == false) {
              tm_init_sntp();
              tm_apply_timesync();
              // TODO : Get the timezone using the region code of the manufacturing data.
              // Currently hardcoding the timezone to be KST-9
              tm_set_timezone("Asia/Seoul");
            } else {
              tm_apply_timesync();
              tm_set_timezone("Asia/Seoul");
            }
            m_timezone_set = 1;
          }
          set_operation_mode(MQTT_START_MODE);
        } else {
          set_operation_mode(TIME_ZONE_SET_MODE);
        }
      } break;
      case MQTT_START_MODE: {
        start_mqttc();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        set_operation_mode(SENSOR_PUB_MODE);
      } break;
      case OTA_FWUPDATE_MODE: {
        // Do not anything while OTA FW updating
        vTaskDelay(pdMS_TO_TICKS(1000));
      } break;
      case SENSOR_PUB_MODE: {
        mqtt_publish_sensor_data();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        stop_mqttc();
        set_operation_mode(SLEEP_MODE);
      } break;
      case SLEEP_MODE: {
        LOGI(TAG, "SLEEP_MODE");
        sleep_timer_wakeup(60);
      } break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void plugged_loop_task(void) {
#if (SENSOR_TEST == 1)
  float random = 0;
  char buf[20] = { 0 };
#else
  int rc = 0;
#if (SENSOR_TYPE == SHT3X)
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
#elif (SENSOR_TYPE == SCD4X)
  char s_co2[20] = { 0 };
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
#elif (SENSOR_TYPE == RK520_02)
  float temperature = 0.0;
  float moisture = 0.0;
  float ec = 0.0;
  char s_temperature[20] = { 0 };
  char s_moisture[20] = { 0 };
  char s_ec[20] = { 0 };
#elif (SENSOR_TYPE == SWSR7500)
  float pyranometer = 0.0;
  char s_pyranometer[20] = { 0 };
#endif
#endif

  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case SENSOR_INIT_MODE: {
#if (SENSOR_TEST == 1)
        set_operation_mode(EASY_SETUP_MODE);
#else
        if ((rc = sensor_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize SHT3x sensor");
        } else {
          set_operation_mode(EASY_SETUP_MODE);
        }
#endif
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_easy_setup_task();
        set_operation_mode(TIME_ZONE_SET_MODE);
      } break;
      case TIME_ZONE_SET_MODE: {
        if (is_device_configured() && is_device_onboard()) {
          // If sensor node onboarded on the network after finishing easy setup.
          // It need to sync the time zone and current time from the ntp server
          if (tm_is_timezone_set() == false) {
            tm_init_sntp();
            tm_apply_timesync();
            // TODO : Get the timezone using the region code of the manufacturing data.
            // Currently hardcoding the timezone to be KST-9
            tm_set_timezone("Asia/Seoul");
          } else {
            tm_apply_timesync();
            tm_set_timezone("Asia/Seoul");
          }
          set_operation_mode(MQTT_START_MODE);
          /* Add file log tesk. */
          start_file_server(8001);

        } else {
          // If the sensor is not connected to the farm network router, it should continuously try to connect.
          set_operation_mode(TIME_ZONE_SET_MODE);
        }
      } break;
      case MQTT_START_MODE: {
        if (is_device_onboard()) {
          start_mqttc();
        }
        set_operation_mode(SENSOR_READ_MODE);
      } break;
      case SENSOR_READ_MODE: {
        if (is_device_onboard()) {
#if (SENSOR_TEST == 1)
          random = (float)(rand() % 10000) / 100;  // 난수 생성
          snprintf(buf, sizeof(buf), "%f", random);
          sysevent_set(I2C_TEMPERATURE_EVENT, buf);
          sysevent_set(I2C_HUMIDITY_EVENT, buf);
          set_operation_mode(SENSOR_PUB_MODE);
#else
#if (SENSOR_TYPE == SHT3X)
          if ((rc = read_temperature_humidity(POWER_OP_MODE, s_temperature, s_humidity)) == CHECK_OK) {
            sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
#elif (SENSOR_TYPE == SCD4X)
          if ((rc = read_co2_temperature_humidity(s_co2, s_temperature, s_humidity)) == CHECK_OK) {
            sysevent_set(I2C_CO2_EVENT, s_co2);
            sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
#elif (SENSOR_TYPE == RK520_02)
          if ((rc = read_soil_ec(s_temperature, s_moisture, s_ec)) == CHECK_OK) {
            ec = atof(s_ec);
            temperature = atof(s_temperature);
            moisture = atof(s_moisture);
            LOGI(TAG, "Power mode > ec : %.2f, moisture : %.2f, temperature : %.2f", ec, moisture, temperature);
            FLOGI(TAG, "Power mode > ec : %.2f, moisture : %.2f, temperature : %.2f", ec, moisture, temperature);
            sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(MB_MOISTURE_EVENT, s_moisture);
            sysevent_set(MB_EC_EVENT, s_ec);
#elif (SENSOR_TYPE == SWSR7500)
          if ((rc = read_solar_radiation(s_pyranometer)) == CHECK_OK) {
            pyranometer = atof(s_pyranometer);
            LOGI(TAG, "pyranometer : %.2f", pyranometer);
            FLOGI(TAG, "pyranometer : %.2f", pyranometer);
            sysevent_set(MB_PYRANOMETER_EVENT, s_pyranometer);
#endif
            set_operation_mode(SENSOR_PUB_MODE);
          } else {
            rc = ERR_SENSOR_READ;
            LOGE(TAG, "sensor read, error = [%d]", rc);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            set_operation_mode(SENSOR_READ_MODE);
          }
#endif
        } else {
          set_operation_mode(SLEEP_MODE);
        }
      } break;
      case SENSOR_PUB_MODE: {
        if (is_device_onboard()) {
          mqtt_publish_sensor_data();
          set_operation_mode(SENSOR_READ_MODE);
        } else {
          set_operation_mode(SLEEP_MODE);
        }
        vTaskDelay(MQTT_SEND_INTERVAL * 1000 / portTICK_PERIOD_MS);
      } break;
      case OTA_FWUPDATE_MODE: {
        // Do not anything while OTA FW updating
        vTaskDelay(pdMS_TO_TICKS(1000));
      } break;
      case SLEEP_MODE: {
        stop_mqttc();
        vTaskDelay(5000 / portTICK_RATE_MS);
        sleep_timer_wakeup(30);
        set_operation_mode(SENSOR_INIT_MODE);
      } break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  int rc = SYSINIT_OK;
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }
  if (m_timezone_set) {
    tm_set_timezone("Asia/Seoul");
  }
  // Start interactive shell command line
  ctx = sc_init();
  if (ctx) {
    sc_start(ctx);
  }

  //#if defined(CONFIG_SMARTFARM_MODBUS_FEATURE)
  //  modbus_sensor_test(3);
  //#endif

  set_device_onboard(0);

  syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);

#if defined(CONFIG_LED_FEATURE)
  create_led_task();
#endif

  if (strcmp(power_mode, "B") == 0) {
    battery_loop_task();
  } else if (strcmp(power_mode, "P") == 0) {
    plugged_loop_task();
  }
}
