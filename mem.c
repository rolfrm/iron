#include <stdint.h>
#include "stdbool.h"
#include "types.h"
#include <stdlib.h>
#include "mem.h"
#include "utils.h"
#include "math.h"
#include <string.h>
#include "log.h"
#include "types.h"
#include "array.h"
__thread allocator * _allocator = NULL;

void with_allocator(allocator * alc, void (* cb)()){
  allocator * old_alloc = _allocator;
  _allocator = alc;
  cb();
  _allocator = old_alloc;
}

void * _alloc(size_t size){
  if(_allocator == NULL) return malloc(size);
  return _allocator->alloc(size);
}

void * alloc0(size_t size){
  void * ptr = _alloc(size);
  memset(ptr,0, size);
  return ptr;
}
void * alloc(size_t size){
  return _alloc(size);
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
  void ** ptrs;
  size_t * ptrs_size;
  size_t cnt; 
  void ** free_ptrs;
  size_t * free_ptrs_size;
  size_t free_cnt;
  
};

void * block_take_free(size_t s, block_chunk * bc){
  while(bc != NULL){
    for(size_t i = 0; i < bc->free_cnt; i++)
      if(bc->free_ptrs_size[i] <= s){
	void * freeptr = bc->free_ptrs[i];
	with_allocator(NULL, lambda(void, (){
	      list_remove((void **) &bc->free_ptrs, &bc->free_cnt, i, sizeof(void *));
	      bc->free_cnt++;
	      list_remove((void **) &bc->free_ptrs_size, &bc->free_cnt, i, sizeof(size_t));
	    }));
	return freeptr;

      }
    bc = bc->last;
  }
  return NULL;
}

void * block_alloc(size_t size){
  block_chunk * balc = _allocator->user_data;
  void * freeptr = block_take_free(size, balc);
  if(freeptr != NULL)
    return freeptr;
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
    size_t start_size = balc->size * 2;
    while(start_size < size) start_size *= 2;
    
    newchunk->block_start = malloc(start_size);
    newchunk->block_front = newchunk->block_start;
    newchunk->size = start_size;
    newchunk->size_left = start_size;
    newchunk->last = balc;
    balc = newchunk;
    _allocator->user_data = balc;
  }

  void * ret = balc->block_front;
  balc->size_left -= size;
  balc->block_front += size;
  with_allocator(NULL, lambda(void, (){
	list_add((void **) &balc->ptrs, &balc->cnt,&ret, sizeof(void *));
	balc->cnt--;
	list_add((void **) &balc->ptrs_size, &balc->cnt,&size, sizeof(size_t));
      }));
  return ret;
}

void block_dealloc(void * ptr){
  block_chunk * balc = _allocator->user_data;
  while(balc != NULL && balc->block_start > ptr)
    balc = balc->last;
  if(balc == NULL) {
    ERROR("No pointer");
    return;
  }

  size_t size = 0;
  for(size_t i = 0; i < balc->cnt; i++){
    if(balc->ptrs[i] == ptr){
      size = balc->ptrs_size[i];
      break;
    }
  }
  with_allocator(NULL, lambda(void, (){
	list_add((void **) &balc->free_ptrs, &balc->free_cnt, &ptr, sizeof(void *));
	balc->free_cnt--;
	list_add((void **) &balc->free_ptrs_size, &balc->free_cnt, &size, sizeof(size_t));
      }));
}

void * block_ralloc(void * ptr, size_t s){
  block_chunk * front = _allocator->user_data;
  block_chunk * balc = front;
  if(balc == NULL || ptr == NULL){
    ASSERT(ptr == NULL);
    return block_alloc(s);
  }
  while(balc != NULL && balc->block_start > ptr)
    balc = balc->last;
  
  ASSERT(balc != NULL);
  for(size_t i = 0; i < balc->cnt; i++){
    logd("%p == %p  | %p\n", ptr, balc->ptrs[i], balc->block_start);
    if(balc->ptrs[i] == ptr){
      size_t sn = balc->ptrs_size[i];
      if(sn >= s){
	return ptr;
      }else{
	if(i < balc->cnt - 1){
	  void * nextptr = balc->ptrs[i];
	  for(size_t j = 0; j < balc->free_cnt; j++){
	    if(balc->free_ptrs[j] == nextptr){
	      //absorb this ptr
	      with_allocator(NULL, lambda(void, (){
		    balc->ptrs_size[i] += balc->free_ptrs_size[j];
		    list_remove((void **) &balc->free_ptrs_size, &balc->free_cnt, j, sizeof(size_t));
		    balc->free_cnt++;
		    list_remove((void **) &balc->free_ptrs, &balc->free_cnt, j, sizeof(void *));
		    list_remove((void **) &balc->ptrs_size, &balc->cnt, j, sizeof(size_t));
		    balc->cnt++;
		    list_remove((void **) &balc->ptrs, &balc->cnt, j, sizeof(void *));
		  }));
	      if(balc->ptrs_size[i] <= s){
		return balc->ptrs[i];
	      }else{
		// sigh..
		return block_ralloc(ptr, s);
	      }
	    }
	  }
	}
	else if(balc->size_left + sn <= s){
	  balc->ptrs_size[i] = s;
	  return ptr;
	}
      }
      void * nptr = block_alloc(s);
      memcpy(nptr, ptr, sn);
      block_dealloc(ptr);
      return nptr;
    }
  }
  logd("balc-cnt: %i %p\n", balc->cnt, ptr);
  ERROR("Should never happen!");
  ASSERT(false);
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
    if(balc->ptrs != NULL)
      list_clean((void **) &balc->ptrs);
    free(balc->block_start);
    free(balc);
    balc = next;
  }
}

void * trace_alloc(size_t size){
  u64 current_size = (u64) _allocator->user_data;
  current_size += 1;
  _allocator->user_data = (void *) current_size;
  return malloc(size);
}

void trace_dealloc(void * ptr){
  _allocator->user_data--;
  free(ptr);
}

void * trace_ralloc(void * ptr, size_t s){
  if(ptr == NULL) return trace_alloc(s);
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



