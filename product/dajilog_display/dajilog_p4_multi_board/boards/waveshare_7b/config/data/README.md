# 제품 데이터 빌드 입력

- `PRODUCT_CATALOG.csv`: 제품 코드·표시명·질소 함량. 처방 환산에 사용한다.
- `PRODUCT_CALIBRATION.csv`: 제품 코드·Lot·A/B/C/D 채널별 전압/분당 배출량. 전압 미리보기와 AUTO 명령에 사용한다.
- 두 CSV의 `product_code`가 동일해야 선택한 제품과 배출량표가 연결된다.
- 두 파일은 CMake configure 때 펌웨어에 포함된다. 수정 후 7B 재빌드와 재플래시가 필요하다.
- 현재 배출량표의 `UNMEASURED` 행은 시험 데이터다. 실제 시비량을 보증하지 않는다.
- 편집 가능한 계산 방식은 `../calculation/dajilog_calculation_policy.h`에 모았다. 단위·식 변경은 검증 후 반영한다.

- 현재 제품명 5개는 모두 한글 표시명이다. Lot은 `SAMPLE-P001-001`~`SAMPLE-P005-001` 예시값이다. 실제 생산 Lot이 아니며 현장 사용 전에 제품 카탈로그와 배출량표의 Lot을 함께 교체한다. P001 시험 배출량표도 동일한 예시 Lot으로 연결했다.
