
#include "EventManager.h"

// 🎯 싱글턴 인스턴스 정의
EventManager& EventManager::GetInstance() {
    static EventManager instance;
    return instance;
}

// 🎯 생성자 (Event Group 초기화)
EventManager::EventManager() {}

// 🎯 소멸자 (Event Group 해제)
EventManager::~EventManager() {
    for (auto& it : eventGroups) {
        vEventGroupDelete(it.second);
    }
}

// 🎯 특정 Event Group 가져오기 (없으면 생성)
EventGroupHandle_t EventManager::GetEventGroup(const std::string& name) {
    if (eventGroups.find(name) == eventGroups.end()) {
        eventGroups[name] = xEventGroupCreate();
    }
    return eventGroups[name];
}

// 🎯 이벤트 비트 설정
void EventManager::SetEventBits(const std::string& name, EventBits_t bits) {
    if (eventGroups.find(name) != eventGroups.end()) {
        ESP_LOGI("EventManager", "Set Event Bits");
        xEventGroupSetBits(eventGroups[name], bits);
    }
}

// 🎯 특정 Event Group의 이벤트 비트 초기화
void EventManager::ClearEventBits(const std::string& name, EventBits_t bits) {
    if (eventGroups.find(name) != eventGroups.end()) {
        ESP_LOGI("EventManager", "Clear Event Bits");
        xEventGroupClearBits(eventGroups[name], bits);
    }
}

// 🎯 이벤트 비트 대기
EventBits_t EventManager::WaitForEventBits(const std::string& name, EventBits_t bits, TickType_t timeout) {
    if (eventGroups.find(name) != eventGroups.end()) {
        ESP_LOGI("EventManager", "Wait for Event Bits");
        return xEventGroupWaitBits(eventGroups[name], bits, pdTRUE, pdFALSE, timeout);
    }
    return 0;
}

// 🎯 특정 Event Group 삭제
void EventManager::RemoveEventGroup(const std::string& name) {
    if (eventGroups.find(name) != eventGroups.end()) {
        vEventGroupDelete(eventGroups[name]);
        eventGroups.erase(name);
    }
}
