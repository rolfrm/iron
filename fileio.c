#include <stdarg.h>
#include <stdio.h>
#include "log.h"
#include "mem.h"
static __thread FILE * outfile = NULL;

size_t stksize = 0;
FILE * ofile[10];

void push_format_out(void * file){
  ofile[stksize++] = file;
  outfile = file;
}

void pop_format_out(){
  stksize--;
  if(stksize > 0)   outfile = ofile[stksize-1];
  else outfile = NULL;
}

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

char * read_stream_to_string(FILE * f){
if(f == NULL)
    return NULL;
  fseek(f,0,SEEK_END);
  size_t size = ftell(f);
  char * buffer = alloc0(size+1);
  fseek(f, 0, SEEK_SET);
  size_t l = fread(buffer,size,1,f);
  ASSERT(l == 1);
  return buffer;
}

char * read_file_to_string(char * filepath){
  FILE * f = fopen(filepath, "r");
  char * data = read_stream_to_string(f);
  fclose(f);
  return data;
}
