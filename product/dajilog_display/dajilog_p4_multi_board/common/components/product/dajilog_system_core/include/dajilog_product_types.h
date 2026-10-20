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
    float output_current;
    float temperature_c;
    uint32_t error_code;
    dajilog_motor_state_t state;
    bool enabled;
    bool online;
} dajilog_motor_snapshot_t;

typedef enum {
    DAJILOG_MANUAL_MOTOR_STOP = 0,
    DAJILOG_MANUAL_MOTOR_FORWARD,
    DAJILOG_MANUAL_MOTOR_REVERSE,
    DAJILOG_MANUAL_MOTOR_SET_RPM,
} dajilog_manual_motor_action_t;

typedef struct {
    uint8_t motor_index;
    uint8_t rpm;
    dajilog_manual_motor_action_t action;
} dajilog_manual_motor_command_t;

typedef struct {
    uint32_t revision;
    int64_t timestamp_us;
    dajilog_operation_state_t operation_state;
    dajilog_work_mode_t work_mode;
    bool can_online;
    dajilog_vehicle_snapshot_t vehicle;
    dajilog_motor_snapshot_t motors[DAJILOG_MOTOR_CHANNEL_COUNT];
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
    DAJILOG_UI_CMD_MOTOR_MANUAL = 0x07020009U,
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
