# DAJILOG ESP32-P4 Multi-Board

Brookesia 0.6 제품 코드 한 벌을 여러 ESP32-P4 보드에서 사용하는 통합 빌드 workspace다.

## 지원 타깃

| 타깃 | BSP | LCD | 칩 설정 | 상태 |
|---|---|---|---|---|
| `waveshare_5` | `waveshare/esp32_p4_wifi6_touch_lcd_5 ==1.0.4` | 720×1280 | rev1.3 | 빌드 확인 |
| `waveshare_7b` | `waveshare/esp32_p4_wifi6_touch_lcd_7b ==3.0.1` | 1024×600 | rev3.0+ | 빌드 확인 |

두 타깃의 bootloader와 펌웨어를 교차 플래시하지 않는다.

## 빌드

ESP-IDF v6.0.2 환경을 활성화한 뒤 workspace 루트에서 실행한다.

```bash
./scripts/build_board.sh waveshare_5
./scripts/build_board.sh waveshare_7b
```

산출물은 `build/<타깃>/`에 분리된다.

```bash
# 연결된 보드에 맞는 타깃만 플래시
idf.py -C boards/waveshare_5 -B ../../build/waveshare_5 -p PORT flash
idf.py -C boards/waveshare_7b -B ../../build/waveshare_7b -p PORT flash
```

`boards/<타깃>/dependencies.lock`도 보드별로 보관한다. BSP나 간접 의존성이
예상치 않게 바뀌지 않도록 새 의존성을 승인해 갱신할 때만 lock 파일을 변경한다.

## 구조

```text
common/components/       여러 보드가 공유하는 앱, 서비스, Brookesia 및 UI/통신 계층
boards/waveshare_5/apps/ 5인치 해상도와 제품에 종속된 EEZ 메인 앱
boards/waveshare_5/      5인치 BSP 의존성, board port와 rev1.3 설정
boards/waveshare_7b/     7B BSP 의존성, board port와 rev3.0 설정 및 향후 전용 앱
scripts/                 타깃별 빌드 도구
```

새 커스텀 보드는 `boards/<보드명>`을 추가하고 `dajilog_board_port`의 동일 API를 구현한다.

## 앱 배치와 선택

- LCD 해상도, 제품 화면 또는 보드 GPIO에 종속된 앱은 `boards/<타깃>/apps/`에 둔다.
- Wi-Fi, OTA, 통신, 상태 관리처럼 재사용하는 기능은 `common/components/`에 둔다.
- 각 target의 `main/Kconfig.projbuild`에서 그 보드에 포함할 앱을 선택한다.
- 5인치 EEZ 앱은 720×1280 전용이므로 7B 빌드에서는 발견되지 않는다.

```bash
idf.py -C boards/waveshare_5 -B ../../build/waveshare_5 menuconfig
idf.py -C boards/waveshare_7b -B ../../build/waveshare_7b menuconfig
```
