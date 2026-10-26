#include <assert.h>
#include <stdio.h>
#include "dajilog_driver_policy.h"

/** @brief Exercise the same pure guards used by the 7B System Task. */
int main(void)
{
    dajilog_driver_setting_t setting = {.driver_index = 0, .mode = 2,
                                        .step = {9, 10}, .fine_decivolt = {3, 1}};
    assert(dajilog_driver_setting_valid(&setting)); /* A=4.8, B=5.1 */
    setting.driver_index = 1;
    assert(dajilog_driver_setting_valid(&setting)); /* C/D use same validation */
    setting.step[0] = 24;
    assert(!dajilog_driver_setting_valid(&setting));
    setting.fine_decivolt[0] = 0;
    assert(dajilog_driver_setting_valid(&setting)); /* 12.0 V inclusive */
    setting.fine_decivolt[1] = 5;
    assert(!dajilog_driver_setting_valid(&setting));
    setting.fine_decivolt[1] = 1;
    setting.mode = 1;
    assert(!dajilog_driver_setting_valid(&setting)); /* no pretend AUTO */
    setting.mode = 0;
    assert(dajilog_driver_setting_valid(&setting));
    setting.driver_index = 2;
    assert(!dajilog_driver_setting_valid(&setting));
    assert(!dajilog_driver_setting_valid(NULL));

    dajilog_motor_snapshot_t feedback = {.online = true, .measured_voltage_valid = true,
        .state = DAJILOG_MOTOR_READY, .measured_timestamp_us = 1000000,
        .status_timestamp_us = 1000000, .fault_timestamp_us = 1000000};
    assert(dajilog_driver_feedback_ready(&feedback, 1000001));
    assert(!dajilog_driver_feedback_ready(&feedback, 4000000)); /* exact expiry */
    assert(!dajilog_driver_feedback_ready(&feedback, 999999)); /* future time */
    feedback.error_code = 1;
    assert(!dajilog_driver_feedback_ready(&feedback, 1000001));
    feedback.error_code = 0;
    feedback.fault_timestamp_us = 0;
    feedback.status_timestamp_us = 0;
    assert(dajilog_driver_feedback_ready(&feedback, 1000001)); /* 66/67 intentionally not polled */
    feedback.measured_voltage_valid = false;
    assert(!dajilog_driver_feedback_ready(&feedback, 1000001));
    feedback.measured_voltage_valid = true;
    feedback.state = DAJILOG_MOTOR_FAULT;
    assert(!dajilog_driver_feedback_ready(&feedback, 1000001));
    feedback.state = DAJILOG_MOTOR_READY;
    feedback.fault_timestamp_us = 1000000;
    feedback.online = false;
    assert(!dajilog_driver_feedback_ready(&feedback, 1000001));
    puts("driver policy: PASS");
    return 0;
}
