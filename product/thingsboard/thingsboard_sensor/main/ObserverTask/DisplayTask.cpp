#include "DisplayTask.h"

DisplayTask::DisplayTask(const std::vector<SubjectType>& subs) : subscribedSubjects(subs), taskName(TAG_DISPLAY) {}

void DisplayTask::run() {
    while (true) {
        ESP_LOGI(TAG_DISPLAY, "DisplayTask running...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void DisplayTask::handleMessage(int msg) {
    ESP_LOGI(TAG_DISPLAY, "DisplayTask handleMessage: %d", msg);
}

std::vector<SubjectType> DisplayTask::getSubscribedSubjectTypes() const {
    return subscribedSubjects;
}

void DisplayTask::onNotify(int data, int subjectId) {
    ESP_LOGI(TAG_DISPLAY, "DisplayTask received data %d from Subject %d", data, subjectId);
}
