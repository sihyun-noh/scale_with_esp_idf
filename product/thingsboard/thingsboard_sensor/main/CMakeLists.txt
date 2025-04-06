set(SOURCES main.cpp led_task.c sensor_task.c battery_task.c led_strip_rmt_sk68xx.c 
                    usb_msc_task.c wifi_thingsboard_send_data.cpp wifi_connect_component.cpp
                    EventManager.cpp TaskManager.cpp 
                    SubjectTask/SubjectTask.cpp
                    SubjectTask/Strategy1.cpp
                    SubjectTask/Strategy2.cpp
                    ObserverTask/GlobalObserverTask.cpp
                    ObserverTask/ControllerTask.cpp
                    ObserverTask/DisplayTask.cpp
                    ObserverTask/NetworkTask.cpp
                    )
idf_component_register(SRCS ${SOURCES}
                    REQUIRES sensor sysevent utils filelog shell usbhost esp-wifi-connect modbus syscore
                    INCLUDE_DIRS . SubjectTask ObserverTask)


##TODO: -fno-rtti란?
# RTTI (Run-Time Type Information, 런타임 타입 정보) 기능을 비활성화하는 GCC/Clang 컴파일러 옵션.
# dynamic_cast, typeid 같은 기능을 사용할 수 없음.
# 임베디드 환경(ESP32 등)에서는 성능과 코드 크기 절약을 위해 기본적으로 RTTI가 비활성화됨.

target_compile_options(${COMPONENT_LIB} PRIVATE -frtti)
