
typedef enum{
  IRON_PROCESS_EXITED = 1,
  IRON_PROCESS_FAULTED = 2,
  IRON_PROCESS_RUNNING = 3
}iron_process_status;

typedef struct{
  int pid;
}iron_process;

int iron_process_run(const char * program, const char ** args, iron_process * out_process);
iron_process_status iron_process_get_status(iron_process proc);
iron_process_status iron_process_wait(iron_process proc, u64 timeout_us);
