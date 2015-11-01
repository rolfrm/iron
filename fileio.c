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
  size_t l =fread(buffer,size,1,f);
  (void)(l);
  //UNUSED(l);//ASSERT(l > 0));
  return buffer;
}

char * read_file_to_string(char * filepath){
  FILE * f = fopen(filepath, "r");
  if(f == NULL) return NULL;
  char * data = read_stream_to_string(f);
  fclose(f);
  return data;
}

int chdir(char * path);

int last_slash(char * path){
  int last_slash_idx = -1;
  {
    char * it = path;
    while(*it != 0){
      if(*it == '/')
	last_slash_idx = it - path;
      it++;
    }
  }
  return last_slash_idx;
}

int enter_dir_of(char * path){ 
  int last_slash_idx = last_slash( path );
  if(last_slash_idx == -1)
    return -1;
  char dirbuf[100];
  sprintf(dirbuf, "%.*s", last_slash_idx, path);
  chdir(dirbuf);
  return 0;
}

int get_filename(char * buffer, char * path){
  int last_slash_idx = last_slash( path );
  if(last_slash_idx == -1)
    last_slash_idx = 0;
  else last_slash_idx += 1;
  path += last_slash_idx;
  while(*path != 0){
    *buffer = *path;
    buffer++;
    path++;
  }
  *buffer = 0;
  return 0;
}
