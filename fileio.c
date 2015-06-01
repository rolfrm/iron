#include <stdarg.h>
#include <stdio.h>
#include "log.h"
#include "mem.h"
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

void write_buffer_to_file(void * buffer, size_t size, char * filepath){
  FILE * f = fopen(filepath, "w");
  if(f == NULL)
    ERROR("Unable to open file '%s'",filepath);
  fwrite(buffer,size,1,f);
  fclose(f);
}

void append_buffer_to_file(void * buffer, size_t size, char * filepath){
  FILE * f = fopen(filepath, "a");
  if(f == NULL)
    ERROR("Unable to open file '%s'",filepath);
  fwrite(buffer,size,1,f);
  fclose(f);
}

char * read_file_to_string(char * filepath){
  FILE * f = fopen(filepath, "r");
  fseek(f,0,SEEK_END);
  size_t size = ftell(f);
  char * buffer = alloc0(size+1);
  fseek(f, 0, SEEK_SET);
  fread(buffer,size,1,f);
  return buffer;
}
