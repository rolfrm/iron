#include <stdint.h>
#include "types.h"
#include <math.h>
double PI = 3.1415926535897932384626433832795;

const f32 f32_pi = 3.1415926535897932384626433832795f;
const f64 f64_pi = 3.1415926535897932384626433832795;
const f32 f32_infinity = 1.0f / 0.0f;
const f32 f32_negative_infinity = -1.0f / 0.0f;
const f64 f64_infinity = 1.0 / 0.0;
const f64 f64_negative_infinity = - 1.0 / 0.0;
const f32 f32_nan = 0.0f / 0.0f;
const f64 f64_nan = 0.0f / 0.0f;
const f32 f32_sqrt2 = 1.4142135;
const f32 f32_sqrt3 = 1.7320508;

int feq(float a, float b, float prec){
  a -= b;
  return a < prec && a > -prec;
}


u8 hibit(u64 x ){
  if(x == 0) return 0;
  return 64 - __builtin_clzll(x) - 1;
}
