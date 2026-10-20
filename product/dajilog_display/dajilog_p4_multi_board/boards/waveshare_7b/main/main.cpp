#include "dajilog_product_start.h"
#include "dajilog_product_controller.h"
#include "dajilog_motor_control.h"
#include "dajilog_speed_sensor.h"

#include "esp_check.h"

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(dajilog_product_controller_register());
    ESP_ERROR_CHECK(dajilog_motor_control_register());
    ESP_ERROR_CHECK(dajilog_speed_sensor_register());
    dajilog_product_start();
}
