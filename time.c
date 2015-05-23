#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include "types.h"
u64 timestamp(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * 1e6 + tv.tv_usec;
}
