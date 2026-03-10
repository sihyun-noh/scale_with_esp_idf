
#include <math.h>
#include <stdbool.h>
#include "math_rpm.h"

#define CAL_POINTS 11  // 2~12V

static const float kVolt[CAL_POINTS] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

// A~D 평균 (2~12V)
static const float kRpmAvg[CAL_POINTS] = { 2.71f,    4.8125f,  7.095f,   10.2725f, 12.875f, 14.925f,
                                           17.7275f, 21.3075f, 23.5475f, 26.075f,  26.025f };

static inline float clampf(float x, float lo, float hi) {
  if (x < lo)
    return lo;
  if (x > hi)
    return hi;
  return x;
}

// 평균 테이블 기준: target_rpm -> voltage
// true: 보간 성공(범위 내), false: 테이블 범위 초과(포화로 처리됨)
bool voltage_for_rpm_avg(float target_rpm, float *out_v) {
  if (!out_v)
    return false;

  // 테이블 내 최대 RPM(비단조 대비)
  float max_rpm = kRpmAvg[0];
  for (int i = 1; i < CAL_POINTS; i++) {
    if (kRpmAvg[i] > max_rpm)
      max_rpm = kRpmAvg[i];
  }
  float min_rpm = kRpmAvg[0];

  // 너무 낮으면 최소 전압
  if (target_rpm <= min_rpm) {
    *out_v = kVolt[0];
    return true;
  }

  // 너무 높으면 최대 전압(정확 보장 X)
  if (target_rpm >= max_rpm) {
    *out_v = kVolt[CAL_POINTS - 1];
    return false;
  }

  // 구간 찾고 선형 보간
  for (int i = 0; i < CAL_POINTS - 1; i++) {
    float r0 = kRpmAvg[i];
    float r1 = kRpmAvg[i + 1];

    float lo = fminf(r0, r1);
    float hi = fmaxf(r0, r1);

    if (target_rpm >= lo && target_rpm <= hi && (hi - lo) > 1e-6f) {
      float v0 = kVolt[i];
      float v1 = kVolt[i + 1];
      float v = v0 + (target_rpm - r0) * (v1 - v0) / (r1 - r0);
      *out_v = clampf(v, kVolt[0], kVolt[CAL_POINTS - 1]);
      return true;
    }
  }

  // 이론상 여기 오면 테이블 비단조/오차가 큰 경우
  *out_v = kVolt[CAL_POINTS - 1];
  return false;
}
