// this does not work with optimizations. unsure why.
// there is a lot of stack magic here..
//#pragma GCC push_options
//#pragma GCC optimize ("O0")

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
struct _costack{
  pthread_attr_t attr;
  void * stack;
  size_t stacksize;
  jmp_buf buf;
};

int pthread_attr_getstack (const pthread_attr_t *__restrict __attr,
				  void **__restrict __stackaddr,
			   size_t *__restrict __stacksize);
int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
			   size_t __stacksize);

static costack ** stacks = NULL;
static size_t stacks_count = 0;

static size_t stk_to_idx(costack * stk){
  for(size_t i = 0 ; i < stacks_count; i++){
    if(stk == stacks[i]) return i;
  }
  return -1;
}

static costack *idx_to_stk(size_t idx){
  return stacks[idx];
}

costack * costack_create(pthread_attr_t * attr){
  costack * cc = malloc(sizeof(costack));
  memset(cc,0,sizeof(costack));
  cc->attr = *attr;
  int nstk = stk_to_idx(NULL);
  if(nstk == -1){
    size_t ncnt = stacks_count == 0 ? 16 : (stacks_count * 2);
    stacks = realloc(stacks,ncnt * sizeof(costack *));
    for(size_t i = stacks_count; i < ncnt; i++)
      stacks[i] = NULL;
    stacks_count = ncnt;
    nstk = stk_to_idx(NULL);
  }
  stacks[nstk] = cc;
  return cc;
}

void checkbuffer(void * data, size_t size){
  volatile char * d = (char *) data;
  char b = 0;
  for(size_t i =0 ; i < size; i++){
    b += d[i];
  }
}

bool costack_save( costack * cc){
  int stk = setjmp(((costack *)cc)->buf) - 1;
  if(stk == -1){
    size_t stacksize =0;
    void * ptr = NULL;
    pthread_attr_getstack(&((costack *)cc)->attr, &ptr, &stacksize);
    if(cc->stack == NULL) 
      cc->stack = malloc(stacksize);
    cc->stacksize = stacksize;
    void * stack = cc->stack;
    memcpy(stack, ptr, stacksize);
    return false;
  }else{
    cc = idx_to_stk(stk);
    size_t stacksize =0;
    void * ptr = NULL;
    pthread_attr_getstack(&cc->attr, &ptr, &stacksize);
    memcpy(ptr,cc->stack, stacksize);
    return true;
  }
}

void costack_resume( costack * stk){
  longjmp(stk->buf,stk_to_idx(stk) + 1);
}

void costack_delete( costack * stk) 
{
  stacks[stk_to_idx(stk)] = NULL;
  free(stk->stack);
}

// CC //
#define CC_INITIALIZED 0
#define CC_YEILDED 1
#define CC_PRE_YEILD 2
#define CC_RESUMED 3
#define CC_ENDED 4
int yeild_states[100] = {CC_INITIALIZED};

costack ** ccstack(){
  static __thread costack * _stk = NULL;
  return &_stk;
}

costack ** mstack(){
  static __thread costack *_stk = NULL;
  return &_stk;
}

void set_current_stack(costack * stk){
  *(ccstack()) = stk;
}

static void yield(){

  costack * stk = *ccstack();
  int idx = stk_to_idx(stk) - 1;
  if(idx < 0 || idx >= 100) return;
  yeild_states[idx] = CC_PRE_YEILD;
  costack_save(stk);
  
  idx = stk_to_idx(stk) - 1;
  if(idx < 0 || idx >= 100) return;
  if(yeild_states[idx] == CC_PRE_YEILD ){
    yeild_states[idx] = CC_YEILDED; 
    costack_resume(*(mstack()));
  }else{
    yeild_states[idx] = CC_RESUMED; 
  }
}

void ccyield(){
  yield();
}

struct _ccdispatch{
  pthread_attr_t attr;
  pthread_t thread;
  costack ** stks;
  u32 stks_count;
  costack * main_stack;
  u32 last_stk;
  void (** loadfcn) (void *);
  void **userptrs;
  sem_t sem;
  sem_t running_sem;
};

static bool inited = false;
#include <GLFW/glfw3.h>
static void * run_dispatcher(void * _attr){
  if(!inited){
    inited = true;
    glfwInit();
  }
  ccdispatch * dis = (ccdispatch *) _attr;
  logd("stks : %i\n", dis->stks_count);
  sem_wait(&dis->sem);
  dis->main_stack = costack_create(&dis->attr);
  *(mstack()) = dis->main_stack;
  costack_save(dis->main_stack);
  
  while(true){
    ASSERT(dis->stks_count > 0);

    if(dis->stks_count == 0) continue;
    
    if(dis->stks_count > dis->last_stk 
       && (yeild_states[dis->last_stk] == CC_RESUMED || yeild_states[dis->last_stk] == CC_ENDED)
       && dis->loadfcn[dis->last_stk] != NULL){
      yeild_states[dis->last_stk] = CC_ENDED;
      dis->loadfcn[dis->last_stk] = NULL;
      dis->stks[dis->last_stk] = NULL;
    }
    
    dis->last_stk++;
    if(dis->last_stk == dis->stks_count){
      sem_post(&dis->running_sem);
      sem_wait(&dis->sem);
      dis->last_stk = 0;
    }
    
    costack * _stk = dis->stks[dis->last_stk];
	
    if(_stk == NULL){
      if(dis->loadfcn[dis->last_stk] == NULL) continue;

      _stk = costack_create(&dis->attr);
      dis->stks[dis->last_stk] = _stk;
      logd("stks 3: %i\n", dis->stks_count);
      set_current_stack(_stk);
      logd("stks 4: %i\n", dis->stks_count);
      dis->loadfcn[dis->last_stk](dis->userptrs[dis->last_stk]);
      logd("stks 5: %i\n", dis->stks_count);
    }else{
      set_current_stack(_stk);
      logd("stks 6: %i\n", dis->stks_count);
      yeild_states[stk_to_idx(_stk) - 1] = CC_ENDED;
      costack_resume(_stk);
    }
   }

  return NULL;
}

void ccthread(ccdispatch * dis, void (*fcn) (void *), void * userdata){
  size_t nidx = dis->stks_count;
  size_t ncnt = dis->stks_count + 1;
  for(size_t i = 0; i < dis->stks_count;i++){
    if(dis->loadfcn[i] == NULL){
      printf("reuse..\n");
      nidx = i;
      break;
    }
  }
  if(nidx == dis->stks_count){
    dis->stks = realloc(dis->stks, ncnt * sizeof(costack *));
    dis->loadfcn = realloc(dis->loadfcn, ncnt * sizeof(void *));
    dis->userptrs = realloc(dis->userptrs, ncnt * sizeof(void *));
  }
  dis->stks[nidx] = NULL;
  yeild_states[dis->last_stk] = CC_INITIALIZED;
  dis->loadfcn[nidx] = fcn;
  dis->userptrs[nidx] = userdata;
  dis->stks_count += 1;
}

ccdispatch * ccstart(){
  ccdispatch * dis = malloc(sizeof(ccdispatch));
  memset(dis,0,sizeof(ccdispatch));
  sem_init(&dis->sem,0, -1);
  sem_init(&dis->running_sem,0, 1);
  int stacksize = 0x4000;
  void * stack = alloc0(stacksize);
  pthread_attr_init(&dis->attr);
  pthread_attr_setstack(&dis->attr,stack,stacksize);
  pthread_create( &dis->thread, &dis->attr, run_dispatcher, dis);
  return dis;
}

void ccstep(ccdispatch * dis){
  logd("stacks: dis->stks_count=%i\n", dis->stks_count);
  if(dis->stks_count == 0) return;
  sem_wait(&dis->running_sem);
  sem_post(&dis->sem);
}

// test / example of use //
void costack_test(){
  void test(void * ptr){
    char * name = (char *) ptr;
    printf("%s: 1\n",name);
    yield();
    printf("%s: 2\n",name);
    yield();
    printf("%s: 3\n",name);
    yield();
  }
  
  ccdispatch * d = ccstart();
  ccthread(d,test,(char *) "Kirk"); 
  ccthread(d,test,(char *) "Clark"); 
  ccthread(d,test,(char *) "Rita"); 
  ccthread(d,test,(char *)"Steve"); 
 for(int i = 0; i < 5; i++){
   ccstep(d);
 }
}

#define get_sp(p) __asm__ volatile ("movq %%rsp, %0" : "=r"(p))
#define get_fp(p) __asm__ volatile ("movq %%rbp, %0" : "=r"(p))
#define set_sp(p) __asm__ volatile ("movq %0, %%rsp" : : "r"(p))
#define set_fp(p) __asm__ volatile ("movq %0, %%rbp" : : "r"(p))

/*static __thread void * stack_ptr, frame_ptr;
static __thread jmp_buf callee_context, caller_context;
static __thread void * current_stack;
enum { WORKING=1, DONE };*/
/*void cc2yield(){
  void * sp = NULL;
  void * fp = NULL;
  get_sp(sp);
  get_fp(fp);
  logd("INNER: %p %p\n", stack_ptr - sp, fp);
  
  if(!setjmp(callee_context)) {
    longjmp(caller_context, WORKING);
  }
}

void cc2run(void (*f)()){

  UNUSED(f);
  void * sp = NULL;
  void * fp = NULL;

  void * old_stack = stack_ptr;
  void * old_frame = frame_ptr;
  
  get_sp(sp);
  get_fp(fp);
  stack_ptr = sp;
  frame_ptr = fp;
  int ret = setjmp(caller_context);
  if(!ret){
    setjmp(caller_context);
  }
  logd("%p %p\n", sp, fp);
  f();
  stack_ptr = old_stack;
  frame_ptr = old_frame;
  }*/
//#pragma GCC pop_options


typedef struct {
  jmp_buf callee_context;
  jmp_buf caller_context;
} coroutine;

typedef void (*func)(void*);

void cc2start(coroutine* c, func f, void* arg, void* sp);
void cc2yield(coroutine* c);
int cc2next(coroutine* c);


enum { WORKING=1, DONE };

void cc2yield(coroutine* c) {
  if(!setjmp(c->callee_context)) {
    longjmp(c->caller_context, WORKING);
  }
}

int cc2next(coroutine* c) {
  int ret = setjmp(c->caller_context);
  if(!ret) {
    longjmp(c->callee_context, 1);
  }
  else {
    return ret == WORKING;
  }
}


typedef struct {
  coroutine* c;
  func f;
  void* arg;
  void* old_sp;
  void* old_fp;
} start_params;

enum { FRAME_SZ=5 }; //fairly arbitrary

void cc2start(coroutine* c, func f, void* arg, void* sp)
{

  start_params* p = ((start_params*)sp)-1;

  //save params before stack switching
  p->c = c;
  p->f = f;
  p->arg = arg;
  get_sp(p->old_sp);
  get_fp(p->old_fp);

  set_sp(p - FRAME_SZ);
  set_fp(p); //effectively clobbers p
  //(and all other locals)
  get_fp(p); //…so we read p back from $fp

  //and now we read our params from p
  if(!setjmp(p->c->callee_context)) {
    set_sp(p->old_sp);
    set_fp(p->old_fp);
    return;
  }
  (*p->f)(p->arg);
  longjmp(p->c->caller_context, DONE);
}

typedef struct _coroutine2{
  jmp_buf callee_context, caller_context;
  void * stack_ptr,  * frame_ptr;

  void * stack_data;
  size_t stack_size;
}coroutine2;


static __thread coroutine2 * current_cc;
coroutine2 * cc3start(void (*f)()){
  coroutine2 * c = alloc0(sizeof(coroutine2));
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

void cc3yield(){
  coroutine2 * c = current_cc;
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

void cc3step(coroutine2 * c){
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
  /*else {
    return ret == WORKING;
    }*/
}
