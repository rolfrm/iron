#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "types.h"
#include "mem.h"
#include "hashtable.h"
#include "log.h"

u32 djb2_hash(char  * str, size_t count)
{
  u32 hash = 5381;
  for(size_t i = 0; i < count; i++){
    let c = str[i];
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

typedef enum {
	      HT_FREE = 0,
	      HT_OCCUPIED = 1,
	      HT_FREED = 2
}ht_state;

typedef struct _hash_table hash_table;
struct _hash_table{
  // this is allowed to be null
  i32 (* hf )(void * key_data, void * userdata);
  // this is allowed to be null
  void * userdata;
  // this is allowed to be null
  bool (* compare ) (void * k1, void * k2, void * userdata);
  
  void * keys;
  void * elems;
  // todo: test performance of having this.
  // alt: have just 8bits of hash value
  // other alt: just do comparison
  
  int * hashes;
  ht_state * occupied;
  size_t capacity;
  size_t count;
  size_t key_size;
  size_t elem_size;
};

void ht_set_hash(hash_table * ht, i32 (* hash)(void * key,  void * userdata)){
  ht->hf = hash;
}

void ht_set_compare(hash_table * ht, bool (* compare)(void * k1, void * k2, void * userdata)){
  ht->compare = compare;
}

i32 default_hash(void * data, size_t size, void * userdata){
  UNUSED(userdata);
  return (i32) djb2_hash(data, size);
}

bool default_compare(void * key_a, void * key_b, size_t size, void * userdata){
  UNUSED(userdata);
  return 0 == memcmp(key_a, key_b, size);
}

hash_table * ht_create2(size_t capacity, size_t key_size, size_t elem_size){
  hash_table * ht = alloc0(sizeof(*ht));
  ht->capacity = capacity;
  ht->key_size = key_size;
  ht->elem_size = elem_size;
  ht->userdata = ht;
  ht->keys = alloc(capacity * key_size);
  ht->elems = alloc(capacity * elem_size);
  ht->occupied = alloc0(capacity * sizeof(ht_state));
  ht->hashes = alloc(capacity * sizeof(int));
  return ht;
}

hash_table * ht_create(size_t key_size, size_t elem_size){
  
  return ht_create2(8, key_size, elem_size);
}


void ht_free(hash_table *ht){

  free(ht->keys);
  free(ht->elems);
  free(ht->occupied);
  free(ht->hashes);
  memset(ht, 0, sizeof(ht[0]));
  free(ht);
}

bool ht_insert(hash_table * ht, void * key, void * nelem);

void ht_grow(hash_table * ht){
  var ht2 = ht_create2(ht->capacity * 2, ht->key_size, ht->elem_size);

  for(u32 i = 0; i < ht->capacity; i++){
    if(ht->occupied[i] == HT_OCCUPIED){
      ht_insert(ht2, ht->keys + i * ht->key_size, ht->elems + i * ht->elem_size);
    }
  }
  SWAP(*ht2, *ht);
  ht_free(ht2);
}

i64 ht_find_free(hash_table * ht, void * key, i64 * freed){
  let key_size = ht->key_size;
  let hash = ht->hf == NULL ? (i32)djb2_hash(key, key_size) : ht->hf(key, ht->userdata); 
  let h2 = hash % ht->capacity;
  let capacity = ht->capacity;
  let compare = ht->compare;

  // find a free slot using linear probing.
  for(size_t _i = 0, i = h2; _i < capacity; _i++, i++){
    if(i >= capacity) i = 0;

    switch(ht->occupied[i]){
    case HT_FREE:
      return i;
    case HT_FREED:
      if(freed != NULL){
	*freed = i;
	freed = NULL;
      }
      FALLTHROUGH;
    case HT_OCCUPIED:
      {
	let thiskey = ht->keys + i * key_size;
	if(ht->hashes[i] == hash){
	  if(compare != NULL && compare(key, thiskey, ht->userdata)){
	    return i;
	  }else if(memcmp(thiskey, key, key_size) == 0){
	    return i;
	  }
	}
      }
    }
  }
  return -1;	 
}

bool ht_insert(hash_table * ht, void * key, void * nelem){
  
  if(ht->count * 2 > ht->capacity)
    ht_grow(ht);
  i64 freed = -1;
  i64 index = ht_find_free(ht, key, &freed);
  let elem_size = ht->elem_size;
  let key_size = ht->key_size;

  let hash = ht->hf == NULL ? (i32)djb2_hash(key, key_size) : ht->hf(key, ht->userdata); 

  // an unoccupied index was found.
  memmove(ht->elems + index * elem_size, nelem, elem_size);
  if(ht->occupied[index] == HT_FREE || freed != -1){
    if(freed != -1)
      index = freed; // reuse a freed bucket
    else
      ht->count += 1;
    memmove(ht->keys + index * key_size, key, key_size);
    ht->occupied[index] = HT_OCCUPIED;
    ht->hashes[index] = hash;
    return true;
  }
  return false;
}

bool ht_lookup(hash_table * ht, void *key, void * out_elem){
  i64 index = ht_find_free(ht, key, NULL);
  let elem_size = ht->elem_size;
  if(ht->occupied[index] == HT_OCCUPIED){
    if(out_elem != NULL)
      memcpy(out_elem, ht->elems + index * elem_size, elem_size);
    return true;
  }
  return false;
}

bool ht_remove(hash_table * ht, void * key){
  i64 index = ht_find_free(ht, key, NULL);
  if(ht->occupied[index] == HT_OCCUPIED){
    ht->occupied[index] = HT_FREED;
    return true;
  }
  return false;
}

void ht_clear(hash_table * ht){
  for(u32 i = 0; i < ht->capacity; i++)
    ht->occupied[i] = HT_FREE;
}

bool ht2_string_test();

bool ht2_test(){


  hash_table * ht2 = ht_create(4, 4);
  int values[] = {1,2,3,1230,32,55,44,33,22,11,111,112,113,114};
  u32 i2 = 2;
  ASSERT(ht_insert(ht2, &values[0], &i2));
  i2 = 0;
  ASSERT(ht_lookup(ht2, &values[0], &i2));
  ASSERT(i2 == 2);
  
  for(u32 i = 0; i < array_count(values); i++){
    ht_insert(ht2, &values[i], &i);
    u32 i2 = 0;
    bool found = ht_lookup(ht2, &values[i], &i2);
    ASSERT(found);
    ASSERT(i2 == i)
      }
  for(u32 i = 0; i < array_count(values); i++){
    u32 i2 = 0;
    bool found = ht_lookup(ht2, &values[i], &i2);
    ASSERT(found);
    ASSERT(i2 == i);
  }

  for(u32 i = 0; i < array_count(values); i+= 2){
    ASSERT(ht_remove(ht2, &values[i]));
  }
  for(u32 i = 0; i < array_count(values); i++){
    bool found = ht_lookup(ht2, &values[i], NULL);
    if((i % 2) == 0){
      ASSERT(!found);
    }else{
      ASSERT(found);
    }
  }
  let count1 = ht2->count;
  for(u32 i = 0; i < array_count(values); i+= 2){
    ASSERT(ht_insert(ht2, &values[i], &i));
  }
  ASSERT(count1 == ht2->count);
  ASSERT(ht2->count == array_count(values));

  ht_clear(ht2);
  for(u32 i = 0; i < array_count(values); i++){
    bool found = ht_lookup(ht2, &values[i], NULL);
    ASSERT(found == false);
  }
  return true;
}

i32 string_hash(void * key, void * userdata){
  UNUSED(userdata);
  return djb2_hash(key, strlen(key));
}

bool string_compare(void * key1, void * key2, void * userdata){
  UNUSED(userdata);
  return strcmp(key1, key2) == 0;
}

hash_table * string_hash_table_new(u32 elem_size){
  hash_table * ht = ht_create(sizeof(char *), elem_size);
  ht_set_hash(ht, string_hash);
  ht_set_compare(ht, string_compare);
  return ht;
}


bool ht2_string_test(){
  hash_table * ht2 = ht_create(8, 4);
  ht_set_hash(ht2, string_hash);
  ht_set_compare(ht2, string_compare);
  char * strings[] = {"A", "C", "asdasd", "11", "22",
		      "ge", "21e2", "BCD",  "DCB", "ASD",
		      "B"};
  for(u32 i = 0; i < array_count(strings); i++){
    ASSERT(ht_insert(ht2, &strings[i], &i));
  }

  char * teststr = "asdasd";
  int idx = 0;
  ASSERT(ht_lookup(ht2, &teststr, &idx));
  ASSERT(idx == 2);
  
  teststr = "B";
  ASSERT(ht_lookup(ht2, &teststr, &idx));
  ASSERT(idx == 10);

  ASSERT(ht_remove(ht2, &teststr));
  
  idx = 0;
  teststr = "B";
  ASSERT(false == ht_lookup(ht2, &teststr, &idx));
  ASSERT(idx == 0);

  return true;
}
