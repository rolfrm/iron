#include <stdio.h>
#include <stdarg.h>

int indent = 0;
void log_print(char * fmt, ...){
  va_list args;
  va_start (args, fmt);
  vprintf (fmt, args);
  va_end (args);
}

