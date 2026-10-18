# 프로젝트 지침 연결 파일

이 프로젝트의 실제 작업 규칙 원본은 다음 Obsidian 문서이다.

```text
/Users/sihyun.noh/Desktop/Obsidian/my_file/LCD_LVGL_ESP_BROOKESIA/AGENTS.md
```

파일 확인, 설계, 코드 수정, 빌드, 플래시 또는 문서 작업 전에 위 문서를 처음부터 끝까지 읽고 적용한다.

추가로 이 프로젝트는 다중 보드 프로젝트다. 공통 제품 코드를 보드 타깃에 복제하지 않고 `common/components`에서 공유한다. BSP, sdkconfig, dependency lock, bootloader 및 build 산출물은 보드별로 격리한다.
