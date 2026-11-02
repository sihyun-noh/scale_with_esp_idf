# 7B 제품·배출량 CSV

실제 빌드 입력은 `config/examples/PRODUCT_CATALOG_TEMPLATE.csv`와 `config/calibration/PRODUCT_CALIBRATION.csv`입니다. `product_code`가 두 파일의 연결 키이며, CSV를 수정한 뒤 7B 펌웨어를 다시 빌드해야 반영됩니다. USB 가져오기는 아직 구현되지 않았습니다.

현재 `P001`의 A–D 채널에는 전압 단계별 배출량 4점씩 등록되어 있습니다. 사용자의 계산값 확인 요청에 따라 이 네 표는 **계산 미리보기용 승인** 상태로 로드됩니다. 저장된 설정의 목표 배출량에서 0.1V 단위 전압을 역보간해 UI에 표시합니다. 이 경로는 모터 CAN 출력과 연결되지 않습니다. 수치는 아직 실측값이 아니므로 CSV의 `data_origin=UNMEASURED`, `product_lot=UNVERIFIED` 표기는 유지합니다. 현장 운전 전 각 비료·로트·채널별 실측값으로 교체해야 합니다. `P002`에는 연결된 배출량표가 없습니다.

`SAMPLE_SYNTHETIC_CALIBRATION_DO_NOT_USE.csv`는 이전 예시 형식을 보존한 참고 파일이며 빌드 입력이 아닙니다. `demo_test.csv`도 빌드 입력이 아닙니다.
