#include "dajilog_motor_control.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dajilog_can_device_manager.h"
#include "dajilog_can_moonwalker.h"
#include "dajilog_system_core.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DRIVER_COUNT 2U
#define CHANNELS_PER_DRIVER 2U

static const char *TAG = "DajilogMotor7B";
static size_t s_driver_indexes[DRIVER_COUNT] = {SIZE_MAX, SIZE_MAX};
static TaskHandle_t s_poll_task;
static bool s_running;
static uint16_t s_commanded_decivolt[DAJILOG_MOTOR_CHANNEL_COUNT];

/** @brief Refresh aggregate running state from all four commanded outputs. */
static void refresh_running_state(void)
{
    s_running = false;
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        if (s_commanded_decivolt[i] != 0U) {
            s_running = true;
            break;
        }
    }
}

/** @brief Map a generated MoonWalker instance to driver slot 0 or 1. */
static size_t driver_slot(const dajilog_can_device_config_t *config)
{
    if (config == NULL || config->instance_name == NULL) {
        return SIZE_MAX;
    }
    if (strcmp(config->instance_name, "motor_driver_1") == 0) {
        return 0U;
    }
    if (strcmp(config->instance_name, "motor_driver_2") == 0) {
        return 1U;
    }
    return SIZE_MAX;
}

/** @brief Convert a valid voltage response to the product decivolt unit. */
static bool voltage_to_decivolt(float voltage, uint16_t *decivolt)
{
    if (decivolt == NULL || !isfinite(voltage) || voltage < 0.0f ||
        voltage > (float)DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX / 10.0f) {
        return false;
    }
    *decivolt = (uint16_t)lroundf(voltage * 10.0f);
    return true;
}

/** @brief Send one typed request to a configured motor driver. */
static esp_err_t send_request(size_t slot,
                              const dajilog_moonwalker_request_t *request)
{
    if (slot >= DRIVER_COUNT || s_driver_indexes[slot] == SIZE_MAX) {
        return ESP_ERR_NOT_FOUND;
    }
    return dajilog_can_device_manager_send(s_driver_indexes[slot], request,
                                           pdMS_TO_TICKS(100));
}

/** @brief Write object 0x0072 for logical channel A, B, C, or D. */
static esp_err_t write_channel_voltage(size_t motor_index,
                                       uint16_t final_decivolt)
{
    if (motor_index >= DAJILOG_MOTOR_CHANNEL_COUNT ||
        final_decivolt > DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    const size_t slot = motor_index / CHANNELS_PER_DRIVER;
    const dajilog_moonwalker_request_t request = {
        .access = DAJILOG_MOONWALKER_WRITE,
        .value_type = DAJILOG_MOONWALKER_FLOAT32,
        .index = DAJILOG_MOONWALKER_INDEX_OUTPUT_COMMAND,
        .sub_index = (uint8_t)(motor_index % CHANNELS_PER_DRIVER) + 1U,
        .value.f32 = (float)final_decivolt / 10.0f,
    };
    return send_request(slot, &request);
}

/** @brief Set all four output channels to zero volts. */
static esp_err_t stop_motor(void *context)
{
    (void)context;
    esp_err_t first_error = ESP_OK;
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        const esp_err_t error = write_channel_voltage(i, 0U);
        if (first_error == ESP_OK && error != ESP_OK) {
            first_error = error;
        }
    }
    if (first_error == ESP_OK) memset(s_commanded_decivolt, 0, sizeof(s_commanded_decivolt));
    refresh_running_state();
    return first_error;
}

/** @brief Validate and send one service-test voltage command. */
static esp_err_t motor_command(const dajilog_motor_voltage_command_t *command,
                               void *context)
{
    (void)context;
    if (command == NULL || command->motor_index >= DAJILOG_MOTOR_CHANNEL_COUNT ||
        command->output_step > DAJILOG_MOTOR_OUTPUT_STEP_MAX ||
        command->final_decivolt > DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    int32_t expected =
        (int32_t)command->output_step * DAJILOG_MOTOR_DECIVOLT_PER_STEP +
        command->trim_decivolt;
    if (expected < 0) {
        expected = 0;
    } else if (expected > DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX) {
        expected = DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX;
    }
    if (command->final_decivolt != (uint16_t)expected) {
        return ESP_ERR_INVALID_ARG;
    }

    dajilog_dashboard_snapshot_t snapshot;
    esp_err_t error = dajilog_system_core_get_snapshot(&snapshot);
    if (error != ESP_OK) {
        return error;
    }
    dajilog_motor_snapshot_t motor = snapshot.motors[command->motor_index];
    if (command->final_decivolt != 0 &&
        (!motor.online || motor.state == DAJILOG_MOTOR_FAULT)) {
        return ESP_ERR_INVALID_STATE;
    }
    error = write_channel_voltage(command->motor_index,
                                  command->final_decivolt);
    if (error == ESP_OK) {
        s_commanded_decivolt[command->motor_index] = command->final_decivolt;
        refresh_running_state();
    }
    return error;
}

/** @brief Report whether a non-zero test output was commanded. */
static bool motor_is_running(void *context)
{
    (void)context;
    return s_running;
}

/** @brief Merge one decoded driver response into its A-D channel snapshot. */
static void update_motor_from_response(size_t slot,
                                       const dajilog_can_device_state_t *state)
{
    /* RX Task owns feedback; never copy stale command fields from System Task. */
    static dajilog_motor_snapshot_t feedback[DAJILOG_MOTOR_CHANNEL_COUNT];
    const dajilog_moonwalker_response_t *response =
        (const dajilog_moonwalker_response_t *)state->decoded;
    if (!state->online) {
        for (size_t local = 0; local < CHANNELS_PER_DRIVER; ++local) {
            const size_t index = slot * CHANNELS_PER_DRIVER + local;
            dajilog_motor_snapshot_t motor = feedback[index];
            motor.online = false;
            motor.state = DAJILOG_MOTOR_OFFLINE;
            motor.measured_voltage_valid = false;
            motor.command_readback_valid = false;
            motor.status_timestamp_us = 0;
            motor.fault_timestamp_us = 0;
            feedback[index] = motor;
            (void)dajilog_system_core_update_motor(index, &motor);
        }
        return;
    }
    if (!response->success || response->sub_index < 1U ||
        response->sub_index > CHANNELS_PER_DRIVER) {
        return;
    }
    const size_t motor_index = slot * CHANNELS_PER_DRIVER +
                               response->sub_index - 1U;
    dajilog_motor_snapshot_t motor = feedback[motor_index];
    motor.enabled = true;
    motor.online = true;

    switch (response->index) {
    case DAJILOG_MOONWALKER_INDEX_OUTPUT_COMMAND: {
        uint16_t value = 0;
        if (!voltage_to_decivolt(response->value.f32, &value)) {
            return;
        }
        motor.command_readback_decivolt = value;
        motor.command_readback_valid = true;
        motor.readback_timestamp_us = esp_timer_get_time();
        motor.command_pending = value != motor.command_decivolt;
        break;
    }
    case DAJILOG_MOONWALKER_INDEX_STATUS:
        motor.status_timestamp_us = esp_timer_get_time();
        motor.driver_status = (uint32_t)response->value.i32;
        motor.state = (motor.driver_status & 0x0004U) != 0U
                          ? DAJILOG_MOTOR_FAULT
                          : ((motor.driver_status & 0x0002U) != 0U
                                 ? DAJILOG_MOTOR_RUNNING
                                 : DAJILOG_MOTOR_READY);
        break;
    case DAJILOG_MOONWALKER_INDEX_FAULT:
        motor.fault_timestamp_us = esp_timer_get_time();
        motor.error_code = (uint32_t)response->value.i32;
        if (motor.error_code != 0U) {
            motor.state = DAJILOG_MOTOR_FAULT;
        }
        break;
    case DAJILOG_MOONWALKER_INDEX_TEMPERATURE:
        motor.temperature_c = response->value.f32;
        break;
    case DAJILOG_MOONWALKER_INDEX_VOLTAGE: {
        uint16_t value = 0;
        if (!voltage_to_decivolt(response->value.f32, &value)) {
            return;
        }
        motor.output_voltage = response->value.f32;
        motor.measured_decivolt = value;
        motor.measured_voltage_valid = true;
        motor.measured_timestamp_us = esp_timer_get_time();
        /* READY here means voltage feedback available, not Fault status verified. */
        if (motor.state == DAJILOG_MOTOR_DISABLED || motor.state == DAJILOG_MOTOR_OFFLINE)
            motor.state = DAJILOG_MOTOR_READY;
        break;
    }
    case DAJILOG_MOONWALKER_INDEX_CURRENT:
        motor.output_current = response->value.f32;
        break;
    case DAJILOG_MOONWALKER_INDEX_VELOCITY:
        motor.actual_rpm = (uint32_t)lroundf(fabsf(response->value.f32));
        break;
    default:
        return;
    }
    feedback[motor_index] = motor;
    (void)dajilog_system_core_update_motor(motor_index, &motor);
}

/** @brief Route a Device Manager notification to driver 1 or driver 2. */
static void motor_observer(size_t index,
                           const dajilog_can_device_config_t *config,
                           const dajilog_can_protocol_profile_t *profile,
                           const dajilog_can_device_state_t *state,
                           void *context)
{
    (void)context;
    if (profile == NULL || state == NULL ||
        strcmp(profile->name, "moonwalker_mdc24") != 0) {
        return;
    }
    const size_t slot = driver_slot(config);
    if (slot >= DRIVER_COUNT) {
        return;
    }
    s_driver_indexes[slot] = index;
    update_motor_from_response(slot, state);
}

/** @brief Poll only 0x0072 command and 0x007A actual voltage for bench testing. */
static void motor_poll(void *context)
{
    (void)context;
    static const struct {
        uint16_t index;
        dajilog_moonwalker_value_type_t type;
    } queries[] = {
        {DAJILOG_MOONWALKER_INDEX_OUTPUT_COMMAND, DAJILOG_MOONWALKER_FLOAT32},
        {DAJILOG_MOONWALKER_INDEX_VOLTAGE, DAJILOG_MOONWALKER_FLOAT32},
    };

    for (;;) {
        for (size_t slot = 0; slot < DRIVER_COUNT; ++slot) {
            for (uint8_t sub = 1U; sub <= CHANNELS_PER_DRIVER; ++sub) {
                for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); ++i) {
                    const dajilog_moonwalker_request_t request = {
                        .access = DAJILOG_MOONWALKER_READ,
                        .value_type = queries[i].type,
                        .index = queries[i].index,
                        .sub_index = sub,
                    };
                    const esp_err_t error = send_request(slot, &request);
                    if (error != ESP_OK) {
                        ESP_LOGD(TAG, "poll D%u index=0x%04X sub=%u failed: %s",
                                 (unsigned)(slot + 1U), request.index,
                                 request.sub_index, esp_err_to_name(error));
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        }
    }
}

/** @brief Discover both configured drivers and start the poll task. */
static esp_err_t motor_start(void *context)
{
    (void)context;
    for (size_t i = 0; i < dajilog_can_device_manager_count(); ++i) {
        const dajilog_can_device_config_t *device = NULL;
        dajilog_can_device_state_t state;
        if (dajilog_can_device_manager_get(i, &device, &state) != ESP_OK ||
            device == NULL) {
            continue;
        }
        const size_t slot = driver_slot(device);
        if (slot < DRIVER_COUNT) {
            s_driver_indexes[slot] = i;
        }
    }
    if (s_driver_indexes[0] == SIZE_MAX || s_driver_indexes[1] == SIZE_MAX) {
        return ESP_ERR_NOT_FOUND;
    }
    if (s_poll_task != NULL) {
        return ESP_OK;
    }
    return xTaskCreate(motor_poll, "motor_can_poll", 4096, NULL, 8,
                       &s_poll_task) == pdPASS
               ? ESP_OK
               : ESP_ERR_NO_MEM;
}

/** @brief Register the product-owned dual-driver voltage backend. */
esp_err_t dajilog_motor_control_register(void)
{
    esp_err_t error = dajilog_can_device_manager_add_observer(
        motor_observer, NULL);
    if (error != ESP_OK) {
        return error;
    }
    const dajilog_motor_backend_t backend = {
        .start = motor_start,
        .command = motor_command,
        .stop = stop_motor,
        .is_running = motor_is_running,
        .context = NULL,
    };
    return dajilog_system_core_register_motor_backend(&backend);
}
