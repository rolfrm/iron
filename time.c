#include <stdlib.h>
#define __USE_POSIX199309 1
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include "types.h"
#include "time.h"
u64 timestamp(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * 1e6 + tv.tv_usec;
}

f128 timestampf(){
  f128 ts = (f128) timestamp();
  return ts * ((f128) 1e-6);
}


u64 measure_elapsed(void (*fcn)(void)){
  u64 t1 = timestamp();
  fcn();
  u64 t2 = timestamp();
  return t2 - t1;
}

void iron_nanosleep(u64 nsec){
  struct timespec ts;
  int res;
  u64 giga = 1000000000;
  ts.tv_sec = nsec / giga;
  ts.tv_nsec = nsec - ts.tv_sec * giga;
  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR); 
}

void iron_usleep(int microseconds){
  iron_nanosleep(microseconds * 1000);
}

void iron_sleep(double seconds){
  u64 us = (u64)(seconds * 1e9);
  iron_usleep(us);
}
