#pragma once

#include <xmmintrin.h>

struct idMath {
  static constexpr float PI = 3.14159265358979323846f;
  static constexpr float TWO_PI = 6.28318530717958647692f;
  static constexpr float M_DEG2RAD = PI / 180.0f;
  static constexpr float M_RAD2DEG = 180.0f / PI;
  static constexpr float FLOAT_EPSILON = 1.192092896e-07f;
  static constexpr float FLT_SMALLEST_NON_DENORMAL = 1.175494351e-38f;
  static float ATan16(float y, float x);
  static float Exp16(float f);
  static float Sin16(float a);
  static float Cos16(float a);
  static void SinCos16(float a, float& s, float& c);
  static float ONE_OVER_TWO_PI;
  static __m128 vector_float_smallestNonDenorm;
};
