#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h> //fsync
#include "log.h"
#include "mem.h"
#include "fileio.h"
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

void format(const char * fmt, ...){
  va_list args;
  va_start (args, fmt);
  vfprintf(outfile == NULL ? stdout : outfile, fmt, args);
  va_end(args);
}

void * get_format_out(){
  return outfile;
}

void write_buffer_to_file_(const void * buffer, size_t size, const char * filepath){
  int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC |O_SYNC, 0660);
  if(fd == 0)
    ERROR("Unable to open file '%s'",filepath);
  write(fd, buffer,size);
  fsync(fd);
  fdatasync(fd);

  close(fd);
}

void write_buffer_to_file(const void * buffer, size_t size, const char * filepath){
  FILE * f = fopen(filepath, "w+");
  if(f == NULL)
    ERROR("Unable to open file '%s'",filepath);
  fwrite(buffer,size,1,f);
  fflush(f);
  fclose(f);
}

void test_buffer_bug(){
#define BUFFER_SIZE 1000
  for(char i = 1; i < 100; i++){
    logd("IT: %i\n", i);
    char set = i;
    {
      char * buffer = alloc0(BUFFER_SIZE);
      buffer[BUFFER_SIZE/2] = set;//memset(buffer, set, sizeof(buffer));
      write_buffer_to_file(buffer, BUFFER_SIZE, "__TEST1__");
      buffer[BUFFER_SIZE/2] = set + 1;
      write_buffer_to_file(buffer, BUFFER_SIZE, "__TEST2__");
      dealloc(buffer);
    }
    {
      size_t s1,s2;
      char * b1 = read_file_to_buffer("__TEST1__", &s1);
      char * b2 = read_file_to_buffer("__TEST2__", &s2);
      ASSERT(s1 == BUFFER_SIZE);
      ASSERT(s2 == BUFFER_SIZE);
      ASSERT(b1[BUFFER_SIZE/2] == set);
      ASSERT(b2[BUFFER_SIZE/2] == set + 1);
      dealloc(b1);dealloc(b2);
    }
  }
  printf("OK!\n");
  #undef BUFFER_SIZE
}

void append_buffer_to_file(const void * buffer, size_t size, const char * filepath){
  FILE * f = fopen(filepath, "a");
  if(f == NULL)
    ERROR("Unable to open file '%s'",filepath);
  fwrite(buffer,size,1,f);
  fclose(f);
}

void * read_stream_to_buffer(FILE * f, size_t * size){
  if(f == NULL)
    return NULL;
  fseek(f,0,SEEK_END);
  *size = ftell(f);
  char * buffer = alloc0(*size);
  fseek(f, 0, SEEK_SET);
  size_t l =fread(buffer,*size,1,f);
  (void)(l);
  //UNUSED(l);//ASSERT(l > 0));
  return buffer;
}

char * read_stream_to_string(FILE * f){
  size_t s;
  return read_stream_to_buffer(f, &s);
}

void * read_file_to_buffer(const char * filepath, size_t * size){
  FILE * f = fopen(filepath, "r");
  if(f == NULL) return NULL;
  char * data = read_stream_to_buffer(f, size);
  fclose(f);
  return data;
}

char * read_file_to_string(const char * filepath){
  size_t size;
  return read_file_to_buffer(filepath, &size);
}



int chdir(const char * path);

int last_slash(const char * path){
  int last_slash_idx = -1;
  {
    char * it = (char *) path;
    while(*it != 0){
      if(*it == '/')
	last_slash_idx = it - path;
      it++;
    }
  }
  return last_slash_idx;
}

int enter_dir_of(const char * path){ 
  int last_slash_idx = last_slash( path );
  if(last_slash_idx == -1)
    return -1;
  char dirbuf[100];
  sprintf(dirbuf, "%.*s", last_slash_idx, path);
  chdir(dirbuf);
  return 0;
}

int get_filename(char * buffer, const char * path){
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
