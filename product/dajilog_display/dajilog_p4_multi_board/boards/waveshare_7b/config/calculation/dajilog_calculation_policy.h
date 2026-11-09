#pragma once

/**
 * @file dajilog_calculation_policy.h
 * @brief D-005: 7B 제품의 처방 환산·분당 목표 시비량·계획 준비량 계산식.
 *
 * 이 파일은 빌드 시 포함하는 계산 정책이다. 제품별 질소 함량은
 * config/data/PRODUCT_CATALOG.csv, 채널별 전압·배출량은
 * config/data/PRODUCT_CALIBRATION.csv에서 가져온다.
 *
 * 여기서 계산하는 것은 목표 또는 계획값이다. 실제 비료 배출 무게를 측정하지 않는다.
 * 공식이나 단위를 변경하면 7B 펌웨어 재빌드와 계산·실기기 회귀 검증이 필요하다.
 * 사용자가 입력한 임의의 수식을 실행하는 런타임 설정 파일은 아니다.
 */

/** @brief 백분율(%)을 비율로 바꿀 때 사용하는 분모. 예: 20% = 20 / 100 = 0.2. */
#define DAJILOG_CALC_PERCENT_BASE 100.0

/** @brief 시간당 이동량을 분당 이동량으로 바꾸는 분모. 1시간 = 60분. */
#define DAJILOG_CALC_MINUTES_PER_HOUR 60.0

/** @brief kg와 g 사이의 단위 변환값. 1kg = 1000g. */
#define DAJILOG_CALC_GRAMS_PER_KG 1000.0

/**
 * @brief [처방 계산] 질소 처방량을 선택 제품의 비료 제품량으로 환산한다.
 *
 * 담당: 질소 기준 처방 모드에서 전층 또는 지중 한 구역의 제품량 계산.
 * 공식: 제품량(kg/10a) = 배정 질소량(kg N/10a) / (제품 N 함량(%) / 100).
 * 예: 배정 질소량 6kg N/10a, 제품 N 함량 20% -> 6 / 0.20 = 30kg/10a.
 * 제품량 직접 입력 모드에서는 이 함수를 사용하지 않는다.
 *
 * @param nitrogen_kg_n_per_10a 해당 구역에 배정한 질소량(kg N/10a).
 * @param nitrogen_pct 제품의 질소 함량(%). 호출 전에 0 초과인지 검증한다.
 * @return 해당 구역에 필요한 비료 제품량(kg/10a).
 */
static inline double dajilog_calc_product_rate(double nitrogen_kg_n_per_10a,
                                               double nitrogen_pct)
{
    return nitrogen_kg_n_per_10a / (nitrogen_pct / DAJILOG_CALC_PERCENT_BASE);
}

/**
 * @brief [운전 목표 계산] 현재 속도에서 구역별 분당 목표 시비량을 계산한다.
 *
 * 담당: S1 전층(A+B) 또는 S2 지중(C+D)의 합계 목표 계산.
 * 공식: 목표(kg/min) = 제품량(kg/10a) * 유효 작업폭(m) * 속도(km/h) / 60.
 * 예: 30kg/10a * 1.2m * 2km/h / 60 = 1.20kg/min.
 * 이후 A/B 또는 C/D 배분율로 채널별 g/min 목표를 만든다. 채널별 명령
 * 전압은 이 함수가 아니라 각 채널의 배출량표에서 별도로 찾는다.
 * 작업 전 미리보기에는 기준속도, AUTO 운전에는 실제 센서 속도를 사용한다.
 * 두둑 길이와 두둑 개수는 이 공식에 곱하지 않는다.
 *
 * @param product_kg_per_10a 해당 구역의 제품 처방량(kg/10a).
 * @param width_m 선택 두둑의 유효 작업폭(m).
 * @param speed_kmh 기준속도 또는 실제 이동속도(km/h).
 * @return 해당 구역의 분당 목표 시비량(kg/min).
 */
static inline double dajilog_calc_target_kg_min(double product_kg_per_10a,
                                                double width_m, double speed_kmh)
{
    return product_kg_per_10a * width_m * speed_kmh /
           DAJILOG_CALC_MINUTES_PER_HOUR;
}

/**
 * @brief [작업 전 계획] 선택 구간 전체에 준비할 비료 계획량을 계산한다.
 *
 * 담당: S1/S2 설정 상세와 Dashboard의 계획 준비량(kg) 표시.
 * 공식: 준비량(kg) = 제품량(kg/10a) * 유효 작업폭(m) * 구간 길이(m) / 1000.
 * 예: 30kg/10a * 1.2m * 80m / 1000 = 2.88kg.
 * 길이는 구간 전체 계획량에만 사용한다. 이 값은 실제 투입량·잔량의 계량값이
 * 아니며, 분당 목표 시비량 계산에도 직접 사용하지 않는다.
 *
 * @param product_kg_per_10a 해당 구역의 제품 처방량(kg/10a).
 * @param width_m 선택 두둑의 유효 작업폭(m).
 * @param length_m 선택 구간의 실제 길이(m).
 * @return 선택 구간의 비료 계획 준비량(kg).
 */
static inline double dajilog_calc_preparation_kg(double product_kg_per_10a,
                                                 double width_m, double length_m)
{
    return product_kg_per_10a * width_m * length_m /
           DAJILOG_CALC_GRAMS_PER_KG;
}
