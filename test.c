#include <unistd.h>
#include <getopt.h>
static const char ** args = NULL;
int cnt = 0;

const char * get_test_opt(const char * name){
  struct option option;
  int idx2;
  int idx = getopt_long(cnt, (char * const *)args, name, &option, &idx2);
  if(idx == -1)
    return NULL;
  return args[idx2];
}

void set_test_opt(const char ** _args, int arg_cnt){
  args = _args;
  cnt = arg_cnt;
}
