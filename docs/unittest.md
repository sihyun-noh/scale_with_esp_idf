# Unit Test for SmartFarm IoT Framework

uinty 라이브러를 사용하여 unit test를 진행하는 과정을 설명 합니다. 

## Project layout
- SMARTFARM-IEF----------------------------- Application project directory
  - package--------------------------------- Components of the application project
    + components..
  - product --------------------------------- Main source files of the application project
  - test -------------------------------------- Test project directory
    + CMakeLists.txt---------------------- Makefiles of the application project

* 레이아웃은 구조에서 package/ 디렉토리에는 실제 프로젝트의 컴포너트 와 각 **컴포너트들의 test 디렉토리**가 포함되어 있습니다. 
* product 와 test 두 프로젝트의 목적은 정상적으로 실행될 때와 단위 테스트를 실행할 때 서로 다른 애플리케이션 동작을 구현하는 것입니다. 
**test 프로젝트는 unit 테스트만 실행하는 간단한 애플리케이션입니다.**

## Unit tests for a component
현재 진행하고 있는 프로젝트의 package에서 wifi_manager의 unit test를 진행하는 과정을 설명합니다.

- SMARTFARM-IEF
  - package                              
    - wifi_manager---------------------------- Components of the application project
      - test---------------------------------- Test directory of the component
        * CMakeLists.txt----------------- Component makefile of tests
        * test_wifi.c---------------------- Test source file
      * CMakeLists.txt---------------------- Component makefile
      * wifi_manager.c--------------------- Component source file

* wifi_manager 컴포너트에 하위폴더 test/를 생성 후 unit test를 진행 할 test case 파일 및 컴포너트 makefile(CMakeLists.txt)를 생성합니다. 
* product에 대한 컴파일 될 때 테스트는 포함되지 않습니다. 테스트 프로젝트는 프로젝트 makefile에서 TEST_COMPONENTS 변수를 설정하여 테스트를 포함합니다.






## How to use test progream
* 진행된 unit test는 특별한 하드웨어가 필요하지 않으며, ESP32 개발 보드에서 실행할 수 있습니다.
* unit test 를 진행하기 위해 esp-idf의 컴포너트 구성 중 **"Unity unit testing library"** 포함 여부를 확인 바랍니다.(idf.py menuconfig)
![image](https://user-images.githubusercontent.com/93233623/172804769-348d5ee6-f54b-4cc6-a60e-5aa34dd97410.png)

## Build and flash
* product 디렉토리 에서 빌드 시 해당 product의 실행파일이 생성됩니다.
* test 디렉토리 에서 빌드 시 unit test를 위한 실행파일이 생성됩니다. 
 - test 디렉토리로 이동 후 빌드 및 unit test 프로그램을 실행합니다. (idf.py -p PORT flash monitor)
* unit test 프로그램이 실행되면 다음과 같은 화면이 시리얼 모니터에 나타 납니다. 
![image](https://user-images.githubusercontent.com/93233623/172808343-2c36497d-b875-495c-8113-c0df216efd7f.png)
* unit test의 항목이 나타나면 해당 **항목의 숫자를 입력 후 Enter key 를 눌러 해당 test를 실행** 합니다.  
* unit test의 시리얼모니터 종료는 "**Ctrl + ]**" 입니다.



