#pragma GCC push_options
#pragma GCC optimize ("O0")
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
#define set_rax() __asm__ volatile("movq 1, %rax")
enum { WORKING=1, DONE = 2 };

typedef struct _coroutine{
  jmp_buf callee_context, caller_context;
  volatile void * stack_ptr,  * frame_ptr;

  void * stack_data;
  size_t stack_size;

  void * fp;
}coroutine;

void cc_get_stack(coroutine * c, void ** out_stack_data, size_t * out_size){
  *out_stack_data = c->stack_data;
  *out_size = c->stack_size;
}

static __thread coroutine * current_cc;
void * cc_get_stack_base(){
  return (void *) current_cc->stack_ptr;
}

coroutine * _ccstart(void (*f)()){
  coroutine * c = alloc0(sizeof(coroutine));
  current_cc = c;
  //get_sp(c->stack_ptr);

    //c->stack_ptr -= sizeof(void *);
  // todo: handle frame pointer

  int r = setjmp(c->caller_context);

  if(!r){
    current_cc = c;
    get_sp(c->stack_ptr);
    f();
    return c;
  }else{
    return c;
  }
}

coroutine * ccstart(void (*f)()){
  long v = 10;
  logd("starting.. %i\n", v);
  return _ccstart(f);
  
}

  
inline void ccyield(){
  static __thread void * sp;
  //coroutine * c = ;
  if(!setjmp(current_cc->callee_context)) {
    get_sp(sp);
    current_cc->stack_size = current_cc->stack_ptr - sp;
    logd("%i \n", current_cc->stack_size);
    ASSERT(((int)current_cc->stack_size) >= 0);
    current_cc->stack_data = realloc(current_cc->stack_data, current_cc->stack_size);
    memcpy(current_cc->stack_data, sp, current_cc->stack_size);
    longjmp(current_cc->caller_context, DONE);
  }else{
    get_sp(sp);
    logd("stack size: %p %p\n", current_cc->stack_size / 4, sp);
    memcpy(sp, current_cc->stack_data, current_cc->stack_size);
  }
}

bool _ccstep(coroutine * c){
  current_cc = c;
  logd("c: %p\n", &c);
  
  //c->stack_ptr = &c - sizeof(c);

  int ret = setjmp(c->caller_context);
  //c->stack_ptr = &ret - sizeof(ret);

  //set_rax(10ULL);
  int i = 0;
  if(!ret) {
    get_sp(c->stack_ptr);
    int * vs;
    get_sp(vs);
    logd("Enter: %p    |", vs);
    for(i = 0; i < 40; i++){
      logd("%5i ", vs[i]);
    }
    logd("\n");
    
    longjmp(c->callee_context, WORKING);
  }else{
    int * vs;
    get_sp(vs);
    logd("resume: %p   |", vs);
    for(i = 0; i < 40; i++){
      logd("%5i ", vs[i]);
    }
    logd("\n");
    //set_fp(c->fp);
    //logd("resume %i %p %p\n", ret, c->fp, &ret);
    //set_fp(c->fp);
    return 0;
  }
}

bool ccstep(coroutine * c){
  long test = 10;
  logd("stepping %i\n", test);
  int r = _ccstep(c);
  logd("Stepped %i\n", r);
  return r;
}
#pragma GCC pop_options

bool rec_step(coroutine * c, int levels){
  if(levels >= 0){
    logd("< %i\n", levels);
    bool v = rec_step(c, levels - 1);
    logd("> %i\n", levels);    
    return v;
  }
  
  return ccstep(c);
  
}

bool coroutine_test(){

  int test_cnt = 0;
  int cc3(){
    u64 x = 5;
    (test_cnt += x++);
    //logd("started %i\n", (test_cnt += x++));
    ccyield();
    (test_cnt += x++);
    //logd("Paused %i\n", (test_cnt += x++));
    ccyield();
    (test_cnt += x++);
    //logd("ended %i\n", (test_cnt += x++));
    return 1;//set_rax();
  }
  void cc1(){
    int step = 4;
    //logd("step 1: %p %i\n", &step, step);
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
  ccstep(c1);

  coroutine * c2 = ccstart(cc2);
  for(int i = 0; i < 3; i++){
    ccstep(c1);
    ccstep(c2);
  }
  logd("Test CNT: %i\n", test_cnt);
  //TEST_ASSERT(test_cnt == 40);

  coroutine * c3 = ccstart((void *)cc3);
  bool r = rec_step(c3, 10);
  logd("== Next step %i\n", r);
  //bool r2 = rec_step(c3,10);
  //bool r3 = rec_step(c3, 5);
  //logd("%i %i %i\n", r, r2, r3);
  
  return TEST_SUCCESS;
}

