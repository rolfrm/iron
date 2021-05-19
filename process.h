
typedef enum{
  IRON_PROCESS_EXITED = 1,
  IRON_PROCESS_FAULTED = 2,
  IRON_PROCESS_RUNNING = 3
}iron_process_status;

typedef struct{
  int pid;
  int stdin_pipe;
  int stdout_pipe;
}iron_process;
// starts a new process. expects that args are NULL terminated.
// returns 0 on success. -1 on error.
int iron_process_run(const char * program, const char ** args, iron_process * out_process);
iron_process_status iron_process_get_status(iron_process proc);
iron_process_status iron_process_wait(iron_process proc, u64 timeout_us);
void iron_process_interupt(iron_process proc);
void iron_process_clean(iron_process * proc);

typedef struct{
  void * data;
}iron_thread;

typedef struct{
  void * data;
}iron_mutex;

iron_thread * iron_start_thread(void * (* fcn)(void * data), void * data);
iron_thread * iron_start_thread0(void (* fcn)(void));
void iron_thread_join(iron_thread * thread);

iron_mutex iron_mutex_create(void);
void iron_mutex_lock(iron_mutex m);
void iron_mutex_unlock(iron_mutex m);
void iron_mutex_destroy(iron_mutex * m);

