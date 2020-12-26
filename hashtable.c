#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "types.h"
#include "mem.h"
#include "hashtable.h"
#include "log.h"

u32 djb2_hash(const char  * str, size_t count)
{
  u32 hash = 5381;
  for(size_t i = 0; i < count; i++){
    let c = str[i];
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

u64 fnv1a_hash(const char * data, size_t count){
  u64 bias = 0xcbf29ce484222325L;
  u64 prime = 0x100000001b3;
  var hash = bias;
  for(size_t i = 0; i < count; i++){
    hash ^= data[i];
    hash *=  prime;
  }
  return hash;
}

u64 hyper_hash(const void * data, size_t count){
  u64 bias = 0xcbf29ce484222325UL;
  u64 prime1 = 0x100000001b3;
  u64 prime2 = 18446744073709551557UL;
  var hash = bias;
  u64 h = 0;
  u64 * d = (u64 *) data;
  u64 c = count / 8;
  size_t rest = count - c * 8;
  if(rest > 0){
    memcpy(&h, data + c * 8, rest);
  }
  size_t i = 0;
  for(; i < c; i++){
    hash ^= d[i];
    hash *= prime1;
    hash *= prime2;
  }
  hash ^= rest;
  hash *= prime1;
  hash *= prime2;
  
  return hash;
}


u32 fnv1a_hash2(const char * data, size_t count){
  return (u32)fnv1a_hash(data, count);
}

u32 hash1(const char * data, size_t count){
  return fnv1a_hash2(data, count);
  //return djb2_hash(data, count);
  //return hyper_hash(data, count);
}

typedef enum {
	      HT_FREE = 0,
	      HT_OCCUPIED = 1,
	      HT_FREED = 2
}ht_state;

typedef struct _hash_table hash_table;
struct _hash_table{
  // this is allowed to be null
  i32 (* hash )(const void * key_data, void * userdata);
  // this is allowed to be null
  void * userdata;
  // this is allowed to be null
  bool (* compare ) (const void * k1, const void * k2, void * userdata);
  
  void * keys;
  void * elems;

  ht_state * occupied;
  size_t capacity;
  size_t count;
  size_t key_size;
  size_t elem_size;
};

void ht_set_hash(hash_table * ht, i32 (* hash)(const void * key,  void * userdata)){
  ht->hash = hash;
}

void ht_set_compare(hash_table * ht, bool (* compare)(const void * k1, const void * k2, void * userdata)){
  ht->compare = compare;
}

i32 default_hash(const void * data, size_t size, void * userdata){
  UNUSED(userdata);
  return (i32) hash1(data, size);
}

bool default_compare(const void * key_a, const void * key_b, size_t size, void * userdata){
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
  return ht;
}

hash_table * ht_create(size_t key_size, size_t elem_size){
  
  return ht_create2(8, key_size, elem_size);
}


void ht_free(hash_table *ht){

  free(ht->keys);
  free(ht->elems);
  free(ht->occupied);
  memset(ht, 0, sizeof(ht[0]));
  free(ht);
}

bool ht_set(hash_table * ht, const void * key, const void * nelem);

static void ht_grow(hash_table * ht){
  var ht2 = ht_create2(ht->capacity * 2, ht->key_size, ht->elem_size);

  ht2->hash = ht->hash;
  ht2->compare = ht->compare;
  ht2->userdata = ht->userdata;
  for(u32 i = 0; i < ht->capacity; i++){
    if(ht->occupied[i] == HT_OCCUPIED){
      ht_set(ht2, ht->keys + i * ht->key_size, ht->elems + i * ht->elem_size);
    }
  }
  SWAP(*ht2, *ht);
  ht_free(ht2);
}

static i64 ht_find_free(const hash_table * ht, const void * key, i64 * freed){
  let key_size = ht->key_size;
  let hash = ht->hash == NULL ? (i32)hash1(key, key_size) : ht->hash(key, ht->userdata);
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
      continue;
    case HT_OCCUPIED:
      {
	let thiskey = ht->keys + i * key_size;
	if(compare != NULL && compare(key, thiskey, ht->userdata)){
	  return i;
	}else if(memcmp(thiskey, key, key_size) == 0){
	  return i;
	}	
      }
    }
  }
  return -1;	 
}

bool ht_set(hash_table * ht, const void * key, const void * nelem){
 
  if(ht->count * 2 > ht->capacity)
    ht_grow(ht);
  i64 freed = -1;
  i64 index = ht_find_free(ht, key, &freed);
  let elem_size = ht->elem_size;
  let key_size = ht->key_size;
  
  // an unoccupied index was found.
  if(ht->occupied[index] == HT_FREE || freed != -1){
    if(freed != -1)
      index = freed; // reuse a freed bucket
    else
      ht->count += 1;
    memmove(ht->elems + index * elem_size, nelem, elem_size);
    memmove(ht->keys + index * key_size, key, key_size);
    ht->occupied[index] = HT_OCCUPIED;
    return true;
  }
  ASSERT(ht->occupied[index] == HT_OCCUPIED);
  memmove(ht->elems + index * elem_size, nelem, elem_size);
  return false;
}

bool ht_get(hash_table * ht, const void *key, void * out_elem){
  i64 index = ht_find_free(ht, key, NULL);
  let elem_size = ht->elem_size;
  if(ht->occupied[index] == HT_OCCUPIED){
    if(out_elem != NULL)
      memcpy(out_elem, ht->elems + index * elem_size, elem_size);
    return true;
  }
  return false;
}

bool ht_remove(hash_table * ht, const void * key){
  i64 index = ht_find_free(ht, key, NULL);
  if(ht->occupied[index] == HT_OCCUPIED){
    ht->occupied[index] = HT_FREED;
    return true;
  }
  return false;
}

bool ht_remove2(hash_table * ht, const void * key, void * out_key, void * out_elem){
  i64 index = ht_find_free(ht, key, NULL);
  if(ht->occupied[index] == HT_OCCUPIED){
    ht->occupied[index] = HT_FREED;
    if(out_key)
      memcpy(out_key, ht->keys + index * ht->key_size, ht->key_size); 
    
    if(out_elem)
      memcpy(out_elem, ht->elems + index * ht->elem_size, ht->elem_size); 

    return true;
  }
  return false;
}

void ht_clear(hash_table * ht){
  for(u32 i = 0; i < ht->capacity; i++)
    ht->occupied[i] = HT_FREE;
}

void ht_iterate(hash_table * ht, void (* it)(void * key, void * elem, void * user_data), void * userdata){
  let key_size = ht->key_size;
  let elem_size = ht->elem_size;
  for(size_t i = 0; i < ht->capacity; i++){
    if(ht->occupied[i] == HT_OCCUPIED){
      it(ht->keys + i * key_size, ht->elems + i * elem_size, userdata);
    }
  }
}

static i32 string_hash(const char ** key, void * userdata){
  UNUSED(userdata);
  u32 h = hash1(*key, strlen(*key));
  return (i32)h;
}

static bool string_compare(const char ** key1, const char ** key2, void * userdata){
  UNUSED(userdata);
  return strcmp(*key1, *key2) == 0;
}

hash_table * ht_create_strkey(size_t elem_size){
  hash_table * ht = ht_create(sizeof(char *), elem_size);
  ht_set_hash(ht, (i32 (*)(const void * a, void * userdata))string_hash);
  ht_set_compare(ht, (bool (*)(const void * a, const void * b, void * userdata))string_compare);
  return ht;
}


bool ht2_string_test();

bool ht2_test(){


  hash_table * ht2 = ht_create(4, 4);
  int values[] = {1,2,3,1230,32,55,44,33,22,11,111,112,113,114};
  u32 i2 = 2;
  ASSERT(ht_set(ht2, &values[0], &i2));
  i2 = 0;
  ASSERT(ht_get(ht2, &values[0], &i2));
  ASSERT(i2 == 2);
  
  for(u32 i = 0; i < array_count(values); i++){
    ht_set(ht2, &values[i], &i);
    u32 i2 = 0;
    bool found = ht_get(ht2, &values[i], &i2);
    ASSERT(found);
    ASSERT(i2 == i)
      }
  for(u32 i = 0; i < array_count(values); i++){
    u32 i2 = 0;
    bool found = ht_get(ht2, &values[i], &i2);
    ASSERT(found);
    ASSERT(i2 == i);
  }

  for(u32 i = 0; i < array_count(values); i+= 2){
    ASSERT(ht_remove(ht2, &values[i]));
  }
  for(u32 i = 0; i < array_count(values); i++){
    bool found = ht_get(ht2, &values[i], NULL);
    if((i % 2) == 0){
      ASSERT(!found);
    }else{
      ASSERT(found);
    }
  }
  let count1 = ht2->count;
  for(u32 i = 0; i < array_count(values); i+= 2){
    ASSERT(ht_set(ht2, &values[i], &i));
  }
  ASSERT(count1 == ht2->count);
  ASSERT(ht2->count == array_count(values));

  ht_clear(ht2);
  for(u32 i = 0; i < array_count(values); i++){
    bool found = ht_get(ht2, &values[i], NULL);
    ASSERT(found == false);
  }
  return true;
}

static void free_keys(void * key, void * elem, void * userdata){
  UNUSED(elem);
  UNUSED(userdata);
  void ** k = (void **) key;
  free(*k);
}

bool ht2_string_test(){
 

  hash_table * ht2 = ht_create_strkey(4);
  char * strings[] = {"A", "C", "asdasd", "11", "22",
		      "ge", "21e2", "BCD",  "DCB", "ASD",
		      "B"};
  for(u32 i = 0; i < array_count(strings); i++){
    char * c = fmtstr("%s", strings[i]); 
    ASSERT(ht_set(ht2, &c, &i));
  }

  char * teststr = "asdasd";
  int idx = 0;
  ASSERT(ht_get(ht2, &teststr, &idx));
  ASSERT(idx == 2);
  
  teststr = "B";
  ASSERT(ht_get(ht2, &teststr, &idx));
  ASSERT(idx == 10);

  ASSERT(ht_remove(ht2, &teststr));
  
  idx = 0;
  teststr = "B";
  ASSERT(false == ht_get(ht2, &teststr, &idx));
  ASSERT(idx == 0);
  ht_iterate(ht2, free_keys, NULL);
  ht_clear(ht2);

  for(int i = 0; i < 2000; i++){
    char * c = fmtstr("%i", i); 
    ASSERT(ht_set(ht2, &c, &i));
  }

  for(int i = 0; i < 2000; i++){
    char * c = fmtstr("%i", i); 
    int i2 = 0;
    ASSERT(ht_get(ht2, &c, &i2));
    ASSERT(i2 == i);
    free(c);	     
  }

  for(int i = 500; i < 1500; i++){
    char * c = fmtstr("%i", i);
    char * outkey;
    int outi;
    ASSERT(ht_remove2(ht2, &c, &outkey, &outi));
    ASSERT( c != outkey);
    ASSERT(strcmp(c, outkey) == 0);
    ASSERT(outi == i);
    free(outkey);
  }


  for(int i = 0; i < 2000; i++){
    char * c = fmtstr("%i", i); 
    int i2 = 0;
    ASSERT(ht_get(ht2, &c, &i2) == (i < 500 || i >= 1500));
    free(c);	     
  }

  for(int i = 500; i < 1500; i++){
    char * c = fmtstr("%i", i); 
    ASSERT(ht_set(ht2, &c, &i));
  }
  
  for(int i = 0; i < 2000; i++){
    char * c = fmtstr("%i", i); 
    int i2 = 0;
    ASSERT(ht_get(ht2, &c, &i2));
    ASSERT(i2 == i);
    free(c);	     
  }

  ht_iterate(ht2, free_keys, NULL);
  ht_clear(ht2);

  
  return true;
}

void hash_table_bench(){
  size_t x = 0xFFFFFFFFFF0;
  hash_table * ht = ht_create(8, 8);
  
  for(size_t i = 0; i < 10000000; i++){
    let x2 = i + x;
    ht_set(ht, &x2, &i);
    
  }  
}

