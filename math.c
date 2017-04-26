#include <stdint.h>
#include <stdlib.h>
#include "types.h"
#include <math.h>
#include <time.h>
#include <stdbool.h>
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

__attribute__ ((const))
int feq(float a, float b, float prec){
  a -= b;
  return a < prec && a > -prec;
}

__attribute__ ((const))
u8 hibit(u64 x ){
  if(x == 0) return 0;
  return 64 - __builtin_clzll(x) - 1;
}

f64 randf64(){
  return (double)rand() / (double)RAND_MAX;
}

f32 randf32(){
  return (float)randf64();
}

static bool inited = false;

u32 randu32(u32 range){
  if(!inited){
    srand(time(NULL));
    inited = true;
  }
  return rand() % range;
}

