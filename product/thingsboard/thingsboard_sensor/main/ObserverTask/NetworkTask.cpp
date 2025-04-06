#include "NetworkTask.h"

NetworkTask::NetworkTask(const std::vector<SubjectType>& subs) : subscribedSubjects(subs), taskName(TAG_NETWORK) {}

void NetworkTask::run() {
    while (true) {
        ESP_LOGI(TAG_NETWORK, "NetworkTask running...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void NetworkTask::handleMessage(int msg) {
    ESP_LOGI(TAG_NETWORK, "NetworkTask handleMessage: %d", msg);
}

std::vector<SubjectType> NetworkTask::getSubscribedSubjectTypes() const {
    return subscribedSubjects;
}

void NetworkTask::onNotify(int data, int subjectId) {
    ESP_LOGI(TAG_NETWORK, "NetworkTask received data %d from Subject %d", data, subjectId);
}
