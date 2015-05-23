#include <stdarg.h>
#include <stdio.h>
static __thread FILE * outfile = NULL;

void with_format_out(void * file, void (* fcn)()){
  FILE * old = outfile;
  outfile = file;
  fcn();
  outfile = old;
}

void format(char * fmt, ...){
  va_list args;
  va_start (args, fmt);
  vfprintf(outfile == NULL ? stdout : outfile, fmt, args);
  va_end(args);
}

void * get_format_out(){
  return outfile;
}
