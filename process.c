#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>  
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <pthread.h>

#include "types.h"
#include "time.h"
#include "log.h"
#include "utils.h"
#include "process.h"
#include "mem.h"

int iron_process_run(const char * program, const char ** args, iron_process * out_process){
  int pipefd[2];

  int out = pipe(pipefd);
  UNUSED(out);
  out_process->stdin_pipe = pipefd[0];
  out_process->stdout_pipe = pipefd[1];
  int pid = fork();
  if(pid == 0){
    dup2(out_process->stdout_pipe, STDOUT_FILENO);
    dup2(out_process->stdout_pipe, STDERR_FILENO);
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
  //logd("Wait.. %i\n", status);
  while(IRON_PROCESS_RUNNING == (status = iron_process_get_status(proc))
	&& (timestamp() - start_time) < timeout_us){
    iron_usleep(timeout_us * 0.05);
  }
  //logd("process took %f s to complete\n", ((double)timestamp() - start_time) / 1000000.0);
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

void iron_process_clean(iron_process * proc){
  kill(proc->pid, SIGSTOP);
  close(proc->stdin_pipe);
  close(proc->stdout_pipe);
  proc->stdin_pipe = -1;
  proc->stdout_pipe = -1;
  proc->pid = -1;
}
void * iron_clone(const void * src, size_t s);
iron_thread * iron_start_thread(void * (* fcn)(void * data), void * data){
  pthread_t tid;
  if(pthread_create( &tid, NULL, fcn, data) != 0)
    return NULL;
  return (iron_thread *) iron_clone(&tid, sizeof(tid));
}

void iron_thread_join(iron_thread * thread){
  pthread_t * tid = (pthread_t *) thread;
  pthread_join(*tid, NULL);
}

iron_mutex iron_mutex_create(){
  iron_mutex mtex;
  mtex.data = alloc0(sizeof(pthread_mutex_t));
  return mtex;
}

void iron_mutex_lock(iron_mutex m){
  pthread_mutex_lock(m.data);
}

void iron_mutex_unlock(iron_mutex m){
  pthread_mutex_unlock(m.data);
}

void iron_mutex_destroy(iron_mutex * m){
  dealloc(m->data);
  memset(m, 0, sizeof(iron_mutex));
}
