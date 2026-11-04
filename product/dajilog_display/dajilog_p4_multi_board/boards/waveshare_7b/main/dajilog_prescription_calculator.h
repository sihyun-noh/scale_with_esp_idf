#pragma once

#include "dajilog_product_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DAJILOG_PRESCRIPTION_CALC_OK = 0,
    DAJILOG_PRESCRIPTION_CALC_MISSING_INPUT,
    DAJILOG_PRESCRIPTION_CALC_PRODUCT_NOT_FOUND,
    DAJILOG_PRESCRIPTION_CALC_INVALID_VALUE,
} dajilog_prescription_calc_status_t;

typedef struct {
    float surface_product_kg_per_10a;
    float subsurface_product_kg_per_10a;
} dajilog_prescription_product_rates_t;

/**
 * @brief 처방 Draft와 선택 제품으로 전층·지중 제품량을 계산한다.
 *
 * NVS, UI, CAN 상태는 변경하지 않는다. 실패하면 output을 0으로 초기화하고
 * 누락/제품 없음/범위 오류를 구분해 반환한다. 결과 단위는 kg/10a이다.
 */
dajilog_prescription_calc_status_t dajilog_prescription_calculate_product_rates(
    const dajilog_prescription_settings_t *prescription,
    const char product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY],
    dajilog_prescription_product_rates_t *output);

#ifdef __cplusplus
}
#endif
