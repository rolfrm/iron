#include <stdint.h>
#include "types.h"
#include <stdlib.h>
#include "mem.h"
#include "utils.h"
#include "log.h"
#include "math.h"
#include <string.h>
__thread allocator * _allocator = NULL;

void with_allocator(allocator * alc, void (* cb)()){
  allocator * old_alloc = _allocator;
  _allocator = alc;
  cb();
  _allocator = old_alloc;
}

void * alloc(size_t size){
  if(_allocator == NULL) return malloc(size);
  return _allocator->alloc(size);
}

void * alloc0(size_t size){
  void * ptr = alloc(size);
  memset(ptr,0, size);
  return ptr;
}

void dealloc(void * ptr){
  if(_allocator == NULL){
    free(ptr);
  }else{
    _allocator->dealloc(ptr);
  }
}

void * ralloc(void * ptr, size_t newsize){
  if(_allocator == NULL){
    return realloc(ptr, newsize); 
  }
  return _allocator->ralloc(ptr,newsize);
}

void * clone(void * src, size_t s){
  void * out = alloc(s);
  memcpy(out, src, s);
  return out;

}

// fun experiment, but it turns out that this is already what it does. so slow...
void * ralloc2(void * ptr, size_t newsize){
  return ralloc(ptr,  2 << hibit(newsize));
}

struct _block_chunk;
typedef struct _block_chunk block_chunk;

struct _block_chunk{
  size_t size;
  size_t size_left;
  void * block_front;
  void * block_start;
  block_chunk * last;
};

void * block_alloc(size_t size){
  block_chunk * balc = _allocator->user_data;

  if(balc == NULL){
block_chunk * newchunk = calloc(1, sizeof(block_chunk));
    size_t start_size = size == 0 ? 128 : size * 8;
    newchunk->block_start = malloc(start_size);
    newchunk->block_front = newchunk->block_start;
    newchunk->size = start_size;
    newchunk->size_left = start_size;
    balc = newchunk;
    _allocator->user_data = balc;
  }

  if(balc->size_left < size){
block_chunk * newchunk = calloc(1, sizeof(block_chunk));
    size_t start_size = balc->last->size * 2;
    while(start_size < size) start_size *= 2;
    
    newchunk->block_start = malloc(start_size);
    newchunk->block_front = balc->block_start;
    newchunk->size = start_size;
    newchunk->size_left = start_size;
    newchunk->last = balc;
    balc = newchunk;
    _allocator->user_data = balc;
  }

  void * ret = balc->block_front;
  balc->size_left -= size;
  balc->block_front += size;
  return ret;
}

void block_dealloc(void * ptr){
  // does nothing.
  UNUSED(ptr);
}

void * block_ralloc(void * ptr, size_t s){
  ERROR("NOT SUPPORTED IN BLOCK ALLOCATOR");
  UNUSED(ptr);
  UNUSED(s);
  return ptr;
}

allocator * block_allocator_make(){
  allocator * alc = malloc(sizeof(allocator));
  alc->alloc = block_alloc;
  alc->dealloc = block_dealloc;
  alc->ralloc = block_ralloc;
  alc->user_data = NULL;
  return alc;
}

void block_allocator_release(allocator * block_allocator){
  block_chunk * balc = block_allocator->user_data;
  while(balc != NULL){
    block_chunk * next = balc->last;
    free(balc->block_start);
    free(balc);
    balc = next;
  }
}

void * trace_alloc(size_t size){
  _allocator->user_data++;
  return malloc(size);
}

void trace_dealloc(void * ptr){
  _allocator->user_data--;
  free(ptr);
}

void * trace_ralloc(void * ptr, size_t s){
  return realloc(ptr,s);
}


allocator * trace_allocator_make(){
  allocator * alc = alloc(sizeof(allocator));
  alc->alloc = trace_alloc;
  alc->dealloc = trace_dealloc;
  alc->ralloc = trace_ralloc;
  alc->user_data = 0;
  return alc;
}

void trace_allocator_release(allocator * aloc){
  dealloc(aloc);
}

size_t trace_allocator_allocated_pointers(allocator * trace_allocator){
  return (size_t) trace_allocator->user_data;
}

#include <stdarg.h>
#include <stdio.h>
#include "fileio.h"
char * fmtstr(char * fmt, ...){
  log("SIZE: ??\n");
  //  va_list args;
  //  va_start (args, msg);
  //  vprintf (msg, args);
  //  va_end (args);
  va_list args;
  va_start (args, fmt);
  size_t size = vsnprintf (NULL, 0, fmt, args) + 1;
  va_end (args);

  char * out = alloc(size);

  va_start (args, fmt);  
  vsprintf (out, fmt, args);
  va_end (args);
  return out;
}
