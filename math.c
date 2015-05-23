#include <stdint.h>
#include "types.h"
#include <math.h>
double PI = 3.1415926535897932384626433832795;

int feq(float a, float b, float prec){
  a -= b;
  return a < prec && a > -prec;
}


u8 hibit(u64 x ){
  if(x == 0) return 0;
  return 64 - __builtin_clzll(x);
}
