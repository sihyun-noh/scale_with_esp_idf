# 7B 제품·배출량 CSV

예시는 이 폴더에 두고 실제 빌드 입력은 `config/data/PRODUCT_CATALOG.csv`와 `config/data/PRODUCT_CALIBRATION.csv`입니다. `product_code`가 두 파일의 연결 키이며, CSV를 수정한 뒤 7B 펌웨어를 다시 빌드해야 반영됩니다. USB 가져오기는 아직 구현되지 않았습니다.

현재 `P001`의 A–D 채널에는 시험용 배출량 점이 등록되어 있습니다. UI 미리보기와 시험용 AUTO CAN 출력 경로에 연결되지만, `data_origin=UNMEASURED`와 `product_lot=UNVERIFIED`이므로 실측 시비량을 보증하지 않습니다. AUTO 시험 출력은 Kconfig의 별도 허용 옵션이 켜져야 합니다. 현장 운전 전 제품·Lot·채널별 실측 승인값으로 교체해야 합니다. `P002`에는 연결된 배출량표가 없습니다.

`SAMPLE_SYNTHETIC_CALIBRATION_DO_NOT_USE.csv`는 이전 예시 형식을 보존한 참고 파일이며 빌드 입력이 아닙니다. `demo_test.csv`도 빌드 입력이 아닙니다.
