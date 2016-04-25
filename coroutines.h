//requires stdbool, pthread.h


// Corutine basics
typedef struct _costack costack;

//costack * costack_create(pthread_attr_t * attr);
bool costack_save( costack * cc);
void costack_resume( costack * stk);
void costack_delete( costack * stk);
void costack_copy(costack * src, costack * dst);

// microthreading

void ccyield();
void ccfork();
void ccend();
typedef struct _ccdispatch ccdispatch;

ccdispatch * ccstart();
void ccthread(ccdispatch * dispatcher, void (* fcn)(void *), void * userdata);
void ccstep(ccdispatch * dispatcher);

typedef struct _coroutine2 coroutine2;
coroutine2 * cc3start(void (*f)());
void cc3yield();
void cc3step(coroutine2 * c);

// test //
void costack_test();

