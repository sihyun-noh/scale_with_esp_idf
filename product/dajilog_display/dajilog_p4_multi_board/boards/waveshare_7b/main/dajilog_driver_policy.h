#pragma once

#include "dajilog_product_types.h"

/** @brief Validate runtime manual/disabled settings without I/O or state mutation. */
static inline bool dajilog_driver_setting_valid(const dajilog_driver_setting_t *setting)
{
    if (!setting || setting->driver_index >= 2 || setting->mode > 2 || setting->mode == 1)
        return false; /* AUTO requires the future calculator/calibration contract. */
    for (unsigned i = 0; i < 2; ++i) {
        if (setting->step[i] > 24 || setting->fine_decivolt[i] > 4 ||
            setting->step[i] * 5U + setting->fine_decivolt[i] > 120U)
            return false;
    }
    return true;
}

/** @brief Voltage-only bench guard; missing 0x66/0x67 is not a fault.
 * This does not certify driver health. Explicit received faults still block output.
 */
static inline bool dajilog_driver_feedback_ready(const dajilog_motor_snapshot_t *motor,
                                                 int64_t now)
{
    return motor && motor->online && motor->measured_voltage_valid &&
           motor->measured_timestamp_us > 0 && now >= motor->measured_timestamp_us &&
           now - motor->measured_timestamp_us < 3000000 &&
           motor->state != DAJILOG_MOTOR_DISABLED &&
           motor->state != DAJILOG_MOTOR_OFFLINE &&
           motor->state != DAJILOG_MOTOR_FAULT && motor->error_code == 0;
}
