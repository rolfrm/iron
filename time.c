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

f128 timestampf(){
  f128 ts = (f128) timestamp();
  return ts * ((f128) 1e-6);
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

void iron_sleep(double seconds){
  int us = (int)(seconds * 1e6);
  usleep(us);
}
