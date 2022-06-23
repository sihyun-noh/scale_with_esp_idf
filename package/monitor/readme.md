# Monitoring Task
 #####  WIFI 상태, heap_memory 상태, Task 상태 등 모니터링 하여 시스템 동작 이슈에 따른 대응 및 보고를 위한 시스템 구성요소 입니다.
 ##### 구성도는 다음과 같습니다.  
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

```

- ### WIFI Monitoring loop
   #### Wifi 모리터링은 STA Connected 와 STA Disconnected를 확인하여 를 WIFI 상태를 보여줍니다. 
   * #### STA Connected 일때 Report message.
            "Conneted SSID" 
   * #### STA Disonnected 일때 Report message.
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
