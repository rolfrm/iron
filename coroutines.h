


typedef struct _coroutine coroutine;

// Calls f and returns the continuation.
coroutine * __attribute__ ((noinline)) ccstart(void (*f)(void));

// resumes the continuation. returns true if it has ended.
bool __attribute__ ((noinline)) ccstep(coroutine * cc);

// suspends the current continuation.
void __attribute__ ((noinline))  ccyield(void);

void * cc_get_stack_base(void);

void cc_get_stack(coroutine * c, void ** out_stack_data, size_t * out_size);

bool coroutine_test(void);
