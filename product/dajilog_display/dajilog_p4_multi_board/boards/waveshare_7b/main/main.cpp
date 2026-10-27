#include "dajilog_product_start.h"
#include "dajilog_product_controller.h"
#include "dajilog_motor_control.h"
#include "dajilog_speed_sensor.h"
#include "dajilog_calibration_catalog.h"

#include "esp_check.h"

/**
 * @brief Register product-specific backends and start the shared product UI.
 *
 * ESP-IDF invokes this entry point once after platform initialization.
 */
extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(dajilog_calibration_catalog_init());
#ifdef CONFIG_DAJILOG_CALIBRATION_SAMPLE_TEST
    ESP_ERROR_CHECK(dajilog_calibration_catalog_load_test_samples());
#endif
    ESP_ERROR_CHECK(dajilog_product_controller_register());
    ESP_ERROR_CHECK(dajilog_motor_control_register());
    ESP_ERROR_CHECK(dajilog_speed_sensor_register());
    dajilog_product_start();
}
