#include <stdint.h>
#include <stdlib.h>
#include "vector.h"
#include "utils.h"

// Vector //
struct _vector{
  size_t elem_size;
  size_t size;
  size_t capacity;
  void * data;
};

vector * vector_create(size_t elem_size){
  vector * vec = malloc(sizeof(vector));
  vec->elem_size = elem_size;
  vec->size = 0;
  vec->capacity = 0;
  vec->data = NULL;
  return vec;
}

void * vector_lookup_unsafe(vector * vec, size_t idx){
  return vec->data + idx * vec->elem_size;
}

void * vector_lookup(vector * vec, size_t idx){
  if(idx >= vec->size)
    return NULL;
  return vector_lookup_unsafe(vec,idx);
}

void vector_rebuffer(vector * vec, size_t new_capacity){
  vec->data = realloc(vec->data, new_capacity * vec->elem_size);
}

void * vector_push(vector * vec, void * buffer, size_t count){
  UNUSED(vec);
  UNUSED(buffer);
  UNUSED(count);
  //while(vec->capacity < vec->size + count){
    
  //}
}

void * vector_pop(vector * vec, size_t count){
  UNUSED(vec);
  UNUSED(count);
}
