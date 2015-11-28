#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include "types.h"

u64 timestamp(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * 1e6 + tv.tv_usec;
}

u64 measure_elapsed(void (*fcn)()){
  u64 t1 = timestamp();
  fcn();
  u64 t2 = timestamp();
  return t2 - t1;
}

void iron_usleep(int microseconds){
  usleep(microseconds);
}
