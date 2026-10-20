#include "dajilog_motor_control.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dajilog_can_device_manager.h"
#include "dajilog_can_moonwalker.h"
#include "dajilog_can_profiles_generated.h"
#include "dajilog_system_core.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MANUAL_MOTOR_INDEX 0U
#define MANUAL_RPM_MAX     30U

static const char *TAG = "DajilogMotor7B";
static size_t s_driver_index = SIZE_MAX;
static TaskHandle_t s_poll_task;
static bool s_running;

static esp_err_t send_user_data(uint8_t sub_index, int32_t value)
{
    if (s_driver_index == SIZE_MAX) {
        return ESP_ERR_NOT_FOUND;
    }
    const dajilog_moonwalker_request_t request = {
        .access = DAJILOG_MOONWALKER_WRITE,
        .value_type = DAJILOG_MOONWALKER_INT32,
        .index = DAJILOG_MOONWALKER_INDEX_USER_DATA,
        .sub_index = sub_index,
        .value.i32 = value,
    };
    return dajilog_can_device_manager_send(
        s_driver_index, &request, pdMS_TO_TICKS(100));
}

static esp_err_t stop_motor(void *context)
{
    (void)context;
    const esp_err_t stop_error = send_user_data(11U, 0);
    const esp_err_t front_error = send_user_data(1U, 0);
    const esp_err_t rear_error = send_user_data(5U, 0);
    s_running = false;
    if (stop_error != ESP_OK) {
        return stop_error;
    }
    return front_error != ESP_OK ? front_error : rear_error;
}

static esp_err_t motor_command(const dajilog_manual_motor_command_t *command,
                               void *context)
{
    (void)context;
    if (command == NULL || command->motor_index != MANUAL_MOTOR_INDEX ||
        command->rpm > MANUAL_RPM_MAX) {
        return ESP_ERR_INVALID_ARG;
    }
    if (command->action == DAJILOG_MANUAL_MOTOR_STOP || command->rpm == 0U) {
        return stop_motor(NULL);
    }

    dajilog_dashboard_snapshot_t snapshot;
    esp_err_t error = dajilog_system_core_get_snapshot(&snapshot);
    if (error != ESP_OK) {
        return error;
    }
    dajilog_motor_snapshot_t motor = snapshot.motors[MANUAL_MOTOR_INDEX];
    if (!motor.online || motor.state == DAJILOG_MOTOR_FAULT) {
        return ESP_ERR_INVALID_STATE;
    }

    error = send_user_data(10U, 0);  // RPM input mode
    if (error == ESP_OK) {
        error = send_user_data(13U, 0);  // Disable common value override
    }
    if (error == ESP_OK) {
        error = send_user_data(1U, command->rpm);  // Front target RPM
    }
    if (error == ESP_OK && command->action != DAJILOG_MANUAL_MOTOR_SET_RPM) {
        const int32_t direction =
            command->action == DAJILOG_MANUAL_MOTOR_REVERSE ? 2 : 1;
        error = send_user_data(2U, direction);
    }
    if (error == ESP_OK && command->action != DAJILOG_MANUAL_MOTOR_SET_RPM) {
        // U11 is a common RUN signal. Keep the unused Rear target at zero.
        error = send_user_data(5U, 0);
    }
    if (error == ESP_OK && command->action != DAJILOG_MANUAL_MOTOR_SET_RPM) {
        error = send_user_data(11U, 1);
        s_running = error == ESP_OK;
    }
    if (error == ESP_OK) {
        motor.target_rpm = command->rpm;
        motor.state = s_running ? DAJILOG_MOTOR_RUNNING : DAJILOG_MOTOR_READY;
        (void)dajilog_system_core_update_motor(MANUAL_MOTOR_INDEX, &motor);
    }
    return error;
}

static bool motor_is_running(void *context)
{
    (void)context;
    return s_running;
}

static void update_motor_from_response(const dajilog_can_device_state_t *state)
{
    const dajilog_moonwalker_response_t *response =
        (const dajilog_moonwalker_response_t *)state->decoded;
    if (!state->online) {
        for (size_t i = 0; i < 2U && i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
            dajilog_dashboard_snapshot_t snapshot;
            if (dajilog_system_core_get_snapshot(&snapshot) != ESP_OK) {
                return;
            }
            dajilog_motor_snapshot_t motor = snapshot.motors[i];
            motor.online = false;
            motor.state = DAJILOG_MOTOR_OFFLINE;
            (void)dajilog_system_core_update_motor(i, &motor);
        }
        return;
    }
    if (!response->success) {
        ESP_LOGW(TAG, "MoonWalker request rejected: error=%u",
                 response->error_code);
        return;
    }
    if (response->sub_index < 1U || response->sub_index > 2U) {
        return;
    }

    const size_t motor_index = response->sub_index - 1U;
    dajilog_dashboard_snapshot_t snapshot;
    if (dajilog_system_core_get_snapshot(&snapshot) != ESP_OK) {
        return;
    }
    dajilog_motor_snapshot_t motor = snapshot.motors[motor_index];
    motor.enabled = true;
    motor.online = true;

    switch (response->index) {
    case DAJILOG_MOONWALKER_INDEX_STATUS:
        motor.driver_status = (uint32_t)response->value.i32;
        if ((motor.driver_status & 0x0004U) != 0U) {
            motor.state = DAJILOG_MOTOR_FAULT;
        } else if ((motor.driver_status & 0x0002U) != 0U) {
            motor.state = DAJILOG_MOTOR_RUNNING;
        } else if ((motor.driver_status & 0x0001U) != 0U) {
            motor.state = DAJILOG_MOTOR_READY;
        } else {
            motor.state = DAJILOG_MOTOR_DISABLED;
        }
        break;
    case DAJILOG_MOONWALKER_INDEX_FAULT:
        motor.error_code = (uint32_t)response->value.i32;
        if (motor.error_code != 0U) {
            motor.state = DAJILOG_MOTOR_FAULT;
        }
        break;
    case DAJILOG_MOONWALKER_INDEX_TEMPERATURE:
        motor.temperature_c = response->value.f32;
        break;
    case DAJILOG_MOONWALKER_INDEX_VOLTAGE:
        motor.output_voltage = response->value.f32;
        break;
    case DAJILOG_MOONWALKER_INDEX_CURRENT:
        motor.output_current = response->value.f32;
        break;
    case DAJILOG_MOONWALKER_INDEX_VELOCITY:
        motor.actual_rpm = (uint32_t)lroundf(fabsf(response->value.f32));
        break;
    default:
        return;
    }
    (void)dajilog_system_core_update_motor(motor_index, &motor);
}

static void motor_observer(size_t index,
                           const dajilog_can_device_config_t *config,
                           const dajilog_can_protocol_profile_t *profile,
                           const dajilog_can_device_state_t *state,
                           void *context)
{
    (void)config;
    (void)context;
    if (profile == NULL || state == NULL ||
        strcmp(profile->name, "moonwalker_mdc24") != 0) {
        return;
    }
    s_driver_index = index;
    update_motor_from_response(state);
}

static void motor_poll(void *context)
{
    (void)context;
    static const struct {
        uint16_t index;
        dajilog_moonwalker_value_type_t type;
    } queries[] = {
        {DAJILOG_MOONWALKER_INDEX_STATUS, DAJILOG_MOONWALKER_INT32},
        {DAJILOG_MOONWALKER_INDEX_FAULT, DAJILOG_MOONWALKER_INT32},
        {DAJILOG_MOONWALKER_INDEX_TEMPERATURE, DAJILOG_MOONWALKER_FLOAT32},
        {DAJILOG_MOONWALKER_INDEX_VOLTAGE, DAJILOG_MOONWALKER_FLOAT32},
        {DAJILOG_MOONWALKER_INDEX_CURRENT, DAJILOG_MOONWALKER_FLOAT32},
        {DAJILOG_MOONWALKER_INDEX_VELOCITY, DAJILOG_MOONWALKER_FLOAT32},
    };

    for (;;) {
        for (uint8_t sub_index = 1U; sub_index <= 2U; ++sub_index) {
            for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); ++i) {
                const dajilog_moonwalker_request_t request = {
                    .access = DAJILOG_MOONWALKER_READ,
                    .value_type = queries[i].type,
                    .index = queries[i].index,
                    .sub_index = sub_index,
                };
                const esp_err_t error = dajilog_can_device_manager_send(
                    s_driver_index, &request, pdMS_TO_TICKS(50));
                if (error != ESP_OK) {
                    ESP_LOGW(TAG,
                             "MoonWalker poll failed: index=0x%04X sub=%u (%s)",
                             request.index, request.sub_index,
                             esp_err_to_name(error));
                }
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}

static esp_err_t motor_start(void *context)
{
    (void)context;
    for (size_t i = 0; i < dajilog_can_device_manager_count(); ++i) {
        const dajilog_can_device_config_t *device = NULL;
        dajilog_can_device_state_t state;
        if (dajilog_can_device_manager_get(i, &device, &state) == ESP_OK &&
            device != NULL &&
            strcmp(DAJILOG_CAN_PROFILES[device->profile_index].name,
                   "moonwalker_mdc24") == 0) {
            s_driver_index = i;
            break;
        }
    }
    if (s_driver_index == SIZE_MAX) {
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
