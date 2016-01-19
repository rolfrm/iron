#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "types.h"
#include "time.h"
#include "log.h"
#include "process.h"

int iron_process_run(const char * program, const char ** args, iron_process * out_process){
  int pid = fork();
  if(pid == 0){
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    int exitstatus = execv(program, (char * const *) args);
    exit(exitstatus == -1 ? 253 : 252);
  }else if(pid < 0)
    return -1;
  out_process->pid = pid;
  return 0;
}

 iron_process_status iron_process_wait(iron_process proc, u64 timeout_us){
  iron_process_status status = iron_process_get_status(proc);
  u64 start_time = timestamp();
  logd("Wait.. %i\n", status);
  while(IRON_PROCESS_RUNNING == (status = iron_process_get_status(proc))
	&& (timestamp() - start_time) < timeout_us){
    iron_usleep(40000);
  }
  logd("process took %f s to complete\n", ((double)timestamp() - start_time) / 1000000.0);
  return status;
}

iron_process_status iron_process_get_status(iron_process proc){
  int status;
  waitpid(proc.pid, &status, WUNTRACED | WNOHANG);
  int exists = kill(proc.pid, 0);
  if(exists != -1)
    return IRON_PROCESS_RUNNING;
  int exit_status = WEXITSTATUS(status);
  return exit_status == 0 ? IRON_PROCESS_EXITED : IRON_PROCESS_FAULTED;
}

void iron_process_interupt(iron_process proc){
  kill(proc.pid, SIGINT);
}
