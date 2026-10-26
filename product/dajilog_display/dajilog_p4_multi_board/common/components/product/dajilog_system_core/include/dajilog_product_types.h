#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_MOTOR_CHANNEL_COUNT 4U
#define DAJILOG_RECIPE_NAME_CAPACITY 32U
#define DAJILOG_ALARM_TEXT_CAPACITY 64U

typedef enum {
    DAJILOG_SETTING_UNSET = 0,
    DAJILOG_SETTING_CONFIGURED,
    DAJILOG_SETTING_CONFIRMED,
} dajilog_setting_readiness_t;

typedef enum {
    DAJILOG_WORK_WIDTH_RIDGE_ONLY = 0,
    DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW,
} dajilog_work_width_mode_t;

typedef enum {
    DAJILOG_PRESCRIPTION_BY_NITROGEN = 0,
    DAJILOG_PRESCRIPTION_BY_PRODUCT_AMOUNT,
} dajilog_prescription_input_mode_t;

enum {
    DAJILOG_RIDGE_PRESENT_COUNT = 1U << 0,
    DAJILOG_RIDGE_PRESENT_WIDTH = 1U << 1,
    DAJILOG_RIDGE_PRESENT_FURROW_WIDTH = 1U << 2,
    DAJILOG_RIDGE_PRESENT_SECTION_LENGTH = 1U << 3,
    DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE = 1U << 4,
};

#define DAJILOG_RIDGE_REQUIRED_MASK                                         \
    (DAJILOG_RIDGE_PRESENT_COUNT | DAJILOG_RIDGE_PRESENT_WIDTH |           \
     DAJILOG_RIDGE_PRESENT_SECTION_LENGTH |                                \
     DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE)

enum {
    DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE = 1U << 0,
    DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET = 1U << 1,
    DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO = 1U << 2,
    DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT = 1U << 3,
    DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT = 1U << 4,
    DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT = 1U << 5,
    DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT = 1U << 6,
    DAJILOG_PRESCRIPTION_PRESENT_MANUAL_SURFACE = 1U << 7,
    DAJILOG_PRESCRIPTION_PRESENT_MANUAL_SUBSURFACE = 1U << 8,
};

typedef struct {
    uint32_t presence_mask;
    uint32_t ridge_count;
    float ridge_width_m;
    float actual_length_m;
    float included_furrow_width_m;
    dajilog_work_width_mode_t work_width_mode;
} dajilog_ridge_settings_t;

typedef struct {
    uint32_t presence_mask;
    dajilog_prescription_input_mode_t input_mode;
    float nitrogen_rate_kg_n_per_10a;
    float surface_nitrogen_ratio_pct;
    float surface_fertilizer_nitrogen_pct;
    float subsurface_fertilizer_nitrogen_pct;
    float surface_product_rate_kg_per_10a;
    float subsurface_product_rate_kg_per_10a;
    float manual_surface_rate_kg_min;
    float manual_subsurface_rate_kg_min;
} dajilog_prescription_settings_t;

typedef struct {
    dajilog_ridge_settings_t ridge;
    dajilog_prescription_settings_t prescription;
    uint32_t ridge_revision;
    uint32_t prescription_revision;
    dajilog_setting_readiness_t draft_readiness;
    dajilog_setting_readiness_t saved_readiness;
    uint32_t saved_revision;
    uint32_t applied_revision;
    bool dirty;
} dajilog_product_settings_snapshot_t;

typedef enum {
    DAJILOG_OPERATION_BOOT = 0,
    DAJILOG_OPERATION_INITIALIZING,
    DAJILOG_OPERATION_READY,
    DAJILOG_OPERATION_RUN_AUTO,
    DAJILOG_OPERATION_RUN_MANUAL,
    DAJILOG_OPERATION_STOPPING,
    DAJILOG_OPERATION_FAULT,
} dajilog_operation_state_t;

typedef enum {
    DAJILOG_WORK_MODE_AUTO = 0,
    DAJILOG_WORK_MODE_MANUAL,
    DAJILOG_WORK_MODE_TEST,
} dajilog_work_mode_t;

typedef enum {
    DAJILOG_MOTOR_DISABLED = 0,
    DAJILOG_MOTOR_OFFLINE,
    DAJILOG_MOTOR_READY,
    DAJILOG_MOTOR_RUNNING,
    DAJILOG_MOTOR_STOPPING,
    DAJILOG_MOTOR_FAULT,
} dajilog_motor_state_t;

typedef enum {
    DAJILOG_ALARM_NONE = 0,
    DAJILOG_ALARM_INFO,
    DAJILOG_ALARM_WARNING,
    DAJILOG_ALARM_CRITICAL,
} dajilog_alarm_severity_t;

typedef struct {
    float speed_kmh;
    float target_speed_kmh;
    uint32_t signal_age_ms;
    bool online;
} dajilog_vehicle_snapshot_t;

typedef struct {
    uint32_t actual_rpm;
    uint32_t target_rpm;
    uint32_t driver_status;
    float target_kg_min;
    float load_pct;
    float output_voltage;
    uint16_t command_decivolt;
    uint16_t command_readback_decivolt;
    uint16_t measured_decivolt;
    bool command_pending;
    bool command_readback_valid;
    bool measured_voltage_valid;
    int64_t measured_timestamp_us;
    int64_t status_timestamp_us;
    int64_t fault_timestamp_us;
    int64_t readback_timestamp_us;
    float output_current;
    float temperature_c;
    uint32_t error_code;
    dajilog_motor_state_t state;
    bool enabled;
    bool online;
} dajilog_motor_snapshot_t;

enum {
    DAJILOG_MOTOR_OUTPUT_STEP_MAX = 24U,
    DAJILOG_MOTOR_DECIVOLT_PER_STEP = 5U,
    DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX = 120U,
};

typedef struct {
    uint8_t motor_index;
    uint8_t output_step;
    int8_t trim_decivolt;
    uint16_t final_decivolt;
} dajilog_motor_voltage_command_t;

/** @brief Applied driver configuration; editing and service tests never mutate it. */
typedef struct {
    uint8_t driver_index;
    uint8_t mode; /* 0: disabled, 1: auto (not implemented), 2: manual */
    uint8_t step[2];
    uint8_t fine_decivolt[2];
    uint32_t revision;
} dajilog_driver_setting_t;

typedef struct {
    uint32_t revision;
    int64_t timestamp_us;
    dajilog_operation_state_t operation_state;
    dajilog_work_mode_t work_mode;
    bool can_online;
    dajilog_vehicle_snapshot_t vehicle;
    dajilog_motor_snapshot_t motors[DAJILOG_MOTOR_CHANNEL_COUNT];
    dajilog_driver_setting_t drivers[2];
    dajilog_alarm_severity_t alarm_severity;
    uint32_t alarm_code;
    char recipe_name[DAJILOG_RECIPE_NAME_CAPACITY];
    char alarm_text[DAJILOG_ALARM_TEXT_CAPACITY];
} dajilog_dashboard_snapshot_t;

typedef enum {
    DAJILOG_UI_CMD_START_AUTO = 0x07020001U,
    DAJILOG_UI_CMD_START_MANUAL = 0x07020002U,
    DAJILOG_UI_CMD_STOP = 0x07020003U,
    DAJILOG_UI_CMD_ACK_ALARM = 0x07020004U,
    DAJILOG_UI_CMD_SETTINGS_APPLY = 0x07020005U,
    DAJILOG_UI_CMD_SETTINGS_DISCARD = 0x07020006U,
    DAJILOG_UI_CMD_CONFIG_IMPORT = 0x07020007U,
    DAJILOG_UI_CMD_CONFIG_EXPORT = 0x07020008U,
    DAJILOG_UI_CMD_MOTOR_VOLTAGE_TEST = 0x07020009U,
    DAJILOG_UI_CMD_DRIVER_APPLY = 0x0702000AU,
} dajilog_ui_command_topic_t;

typedef enum {
    DAJILOG_UI_STATE_DASHBOARD_SNAPSHOT = 0x07021001U,
    DAJILOG_UI_STATE_COMMAND_RESULT = 0x07021002U,
    DAJILOG_UI_STATE_SETTINGS_RESULT = 0x07021003U,
    DAJILOG_UI_STATE_STORAGE_PROGRESS = 0x07021004U,
} dajilog_ui_state_topic_t;

#ifdef __cplusplus
}
#endif
