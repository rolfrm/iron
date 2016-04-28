
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
//#include "full.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include "mem.h"
#include "types.h"
#include "coroutines.h"
#include "log.h"
#include "utils.h"
#include "test.h"


#define get_sp(p) __asm__ volatile ("movq %%rsp, %0" : "=r"(p))
#define get_fp(p) __asm__ volatile ("movq %%rbp, %0" : "=r"(p))
#define set_sp(p) __asm__ volatile ("movq %0, %%rsp" : : "r"(p))
#define set_fp(p) __asm__ volatile ("movq %0, %%rbp" : : "r"(p))

enum { WORKING=1, DONE };

typedef struct _coroutine{
  jmp_buf callee_context, caller_context;
  void * stack_ptr,  * frame_ptr;

  void * stack_data;
  size_t stack_size;
}coroutine;

void cc_get_stack(coroutine * c, void ** out_stack_data, size_t * out_size){
  *out_stack_data = c->stack_data;
  *out_size = c->stack_size;
}

static __thread coroutine * current_cc;
void * cc_get_stack_base(){
  return current_cc->stack_ptr;
}

coroutine * ccstart(void (*f)()){
  coroutine * c = alloc0(sizeof(coroutine));
  current_cc = c;
  get_sp(c->stack_ptr);
  get_fp(c->frame_ptr);
  //set_sp(c->stack_ptr - 5);
  int r = setjmp(c->caller_context);

  //logd("R: %i \n", r);
  if(!r){
    current_cc = c;
    f();
  }
  return c;
}

void ccyield(){
  coroutine * c = current_cc;
  if(!setjmp(c->callee_context)) {
    void * sp;
    get_sp(sp);
    size_t call_stack_size = c->stack_ptr - sp;
    //logd("stack size: %i\n", call_stack_size);
    int _i = call_stack_size;
    ASSERT(_i > 0);
    c->stack_data = realloc(c->stack_data, (c->stack_size = call_stack_size));
    memcpy(c->stack_data, sp, c->stack_size);
    /*int * d = c->stack_data;
    for(u32 i = 0; i < call_stack_size / sizeof(int); i++){
      logd("%i ", d[i]);
      }*/
    //logd("\nresume %i\n", c->caller_context);
    longjmp(c->caller_context, DONE);
  }else{
    c = current_cc;
    void * sp;
    get_sp(sp);
    //logd("resuming..\n");
    memcpy(sp, c->stack_data, c->stack_size);
    //logd("resumed..\n");
  }
}

bool ccstep(coroutine * c){
  current_cc = c;
  int ret = setjmp(c->caller_context);
  if(!ret) {
    //logd("Resume!\n");
    void * sp;
    get_sp(sp);
    //memcpy(sp- c->stack_size - 8, c->stack_data, c->stack_size);
    /*int * d = c->stack_data;
    for(u32 i = 0; i < c->stack_size / sizeof(int); i++){
      logd("%i ", d[i]);
    }
    logd("\n");*/
    longjmp(c->callee_context, WORKING);
  }
  return false;
}

bool coroutine_test(){

  int test_cnt = 0;
  void cc1(){
    int step = 4;
    logd("step 1: %p %i\n", &step, step);
    while(true){
      test_cnt += step;
      ccyield();
    }
  }

  void cc2(){
    int step = 6;
    logd("step 2: %p %i\n", &step, step);
    while(true){
      test_cnt += step;
      ccyield();
    }
  }
  coroutine * c1 = ccstart(cc1);
  coroutine * c2 = ccstart(cc2);
  for(int i = 0; i < 3; i++){
    ccstep(c1);
    ccstep(c2);
  }
  TEST_ASSERT(test_cnt == 40);
  
  return TEST_SUCCESS;
}
