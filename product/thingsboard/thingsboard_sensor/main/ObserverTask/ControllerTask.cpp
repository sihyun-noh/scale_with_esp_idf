
#include "ControllerTask.h"

ControllerTask::ControllerTask(const std::vector<SubjectType>& subs)
    : subscribedSubjects(subs), taskName(TAG_CONTROLLER) {}

void ControllerTask::run() {
    while (true) {
        ESP_LOGI(TAG_CONTROLLER, "ControllerTask running...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void ControllerTask::handleMessage(int msg) {
    ESP_LOGI(TAG_CONTROLLER, "ControllerTask handleMessage: %d", msg);
}

std::vector<SubjectType> ControllerTask::getSubscribedSubjectTypes() const {
    return subscribedSubjects;
}

void ControllerTask::onNotify(int data, int subjectId) {
    ESP_LOGI(TAG_CONTROLLER, "ControllerTask received data %d from Subject %d", data, subjectId);
}
