#include "Strategy1.h"
#include <iostream>
#include "EventManager.h"
#include "sysevent.h"
#include "esp_log.h"
#include "syslog.h"
#include "gpio_api.h"
#include "config.h"
#include "main.h"

#define TAG "Strategy1"
#ifdef __cplusplus
extern "C" {
#endif
extern int sensor_init(void);
extern int sensor_read(void);
#ifdef __cplusplus
}
#endif

static int sensor_power_on() {
    return gpio_write(SENSOR_POWER_CONTROL_PORT, 1);
}
static int sensor_power_off() {
    return gpio_write(SENSOR_POWER_CONTROL_PORT, 0);
}

bool Strategy1::isInitialized = false;

void Strategy1::execute() {
    int rc = 0;

    // 센서 초기화 부분
    LOGI(TAG, "SENSOR_INIT_MODE");
    if (!isInitialized) {
        if ((rc = sensor_init()) != SYSINIT_OK) {
            LOGI(TAG, "Could not initialize sensor");
            return;
        }
        isInitialized = true;
    }

    // 센서 읽기 부분
    LOGI(TAG, "SENSOR_READ_MODE");
    sensor_power_on();
    // sensor warming up time
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    if (sensor_read() == CHECK_OK) {
        EventManager::GetInstance().SetEventBits(EVENT_GROUPS_FOR_THINGSBOARD, EVENT_BIT_TASK_READY);
    } else {
        rc = ERR_SENSOR_READ;
        LOGE(TAG, "sensor read, error = [%d]", rc);
        EventManager::GetInstance().SetEventBits(EVENT_GROUPS_FOR_THINGSBOARD, EVENT_BIT_TASK_READY);
    }

    sensor_power_off();
}
