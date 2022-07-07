# Monitoring Task
 #####  WIFI ����, heap_memory ����, Task ���� �� ����͸� �Ͽ� �ý��� ���� �̽��� ���� ���� �� ���� ���� �ý��� ������� �Դϴ�.
 ##### �������� ������ �����ϴ�.  
```mermaid
stateDiagram
[*] --> Monitoring_Task
Monitoring_Task --> WIFI_Monitoring
Monitoring_Task --> Heap_Monitoring
Monitoring_Task --> Task_Monitoring
WIFI_Monitoring --> STA_Disonnected
WIFI_Monitoring --> STA_Connected
    STA_Connected --> OK
    OK --> Report      
    STA_Connected --> FAIL
    FAIL --> Report
    STA_Disonnected --> Reason_Code_Check
    Reason_Code_Check --> Report  
    Report --> [*] 
Heap_Monitoring --> critical_level
Heap_Monitoring --> warning_level
critical_level --> heap_alarm_event_set
warning_level --> heap_alarm_event_set
heap_alarm_event_set --> [*]

is_run_task_moni_alarm --> Task_Monitoring
is_run_task_heart_bit --> Task_Monitoring
Task_Monitoring --> Task_run_count_check
Task_run_count_check -->tesk_alarm_event_set
tesk_alarm_event_set -->[*]
```

- ### WIFI Monitoring loop
   #### Wifi ���͸��� STA Connected �� STA Disconnected�� Ȯ���Ͽ� �� WIFI ���¸� �����ݴϴ�. 
   * #### STA Connected �϶� Report message.
            "Conneted SSID" 
   * #### STA Disonnected �϶� Report message.
            "UNSPECIFIED"
            "AUTH_EXPIRE"
            "AUTH_LEAVE"
            "ASSOC_EXPIRE"
            "ASSOC_TOOMANY"
            "NOT_AUTHED"
            "NOT_ASSOCED"
            "ASSOC_LEAVE"
            "ASSOC_NOT_AUTHED"
            "DISASSOC_PWRCAP_BAD"
            "DISASSOC_SUPCHAN_BAD"
            "UNSPECIFIED"
            "IE_INVALID"
            "MIC_FAILURE"
            "4WAY_HANDSHAKE_TIMEOUT"
            "GROUP_KEY_UPDATE_TIMEOUT"
            "IE_IN_4WAY_DIFFERS"
            "GROUP_CIPHER_INVALID"
            "PAIRWISE_CIPHER_INVALID"
            "AKMP_INVALID"
            "UNSUPP_RSN_IE_VERSION"
            "INVALID_RSN_IE_CAP"
            "802_1X_AUTH_FAILED"
            "CIPHER_SUITE_REJECTED"
            "BEACON_TIMEOUT"
            "NO_AP_FOUND"
            "AUTH_FAIL"
            "ASSOC_FAIL"
            "HANDSHAKE_TIMEOUT"
            "CONNECTION_FAIL"
- ### Heap Monitoring loop
   #### ���޸� ���¸� Ȯ���Ͽ� Memory leck �߻��� sysevent�� �߻� ��ŵ�ϴ�.
   EVENT : 
   HEAP_CRITICAL_LEVEL_WARNING_EVENT(2048) HEAP_WARNING_LEVEL_WARNING_EVENT(1014)

- ### Task Monitoring loop
   #### ������ Task�� ���¸� Ȯ�� �մϴ�. ���θ� ����� Task�� is_run_task_moni_alarm()���� Task�� Task_Monitoring�� ����ϰ�, is_run_task_heart_bit()���� Task loop alive ���¸� �����մϴ�. Task loop alive �� �߻����� ���� �� sysevent�� �߻� ��ŵ�ϴ�.
   EVENT : TASK_MONITOR_EVENT
