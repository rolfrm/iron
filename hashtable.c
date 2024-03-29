#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include "types.h"
#include "log.h"
#include "hashtable.h"
bool debug_set = false;

void ht_set_alloc(hash_table * ht, void * (* alloc)(size_t c), void (* free)(void * ptr)){
  
  ht->alloc_keys = alloc;
  ht->alloc_values = alloc;
  ht->alloc_state = alloc;

  ht->free_keys = free;
  ht->free_values = free;
  ht->free_state = free;
  ht->alloc = alloc;
  ht->free = free;

}

u32 djb2_hash(const char  * str, size_t count)
{
  u32 hash = 5381;
  for(size_t i = 0; i < count; i++){
    u8 c = str[i];
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

static inline u64 fnv1a_hash(const char * data, size_t count){
  u64 bias = 0xcbf29ce484222325L;
  u64 prime = 0x100000001b3;
  u64 hash = bias;
  for(size_t i = 0; i < count; i++){
    hash ^= data[i];
    hash *=  prime;
  }
  return hash;
}

static inline u64 hash64(u64 * bytes, size_t _){
  UNUSED(_);
  //return fnv1a_hash(bytes, 8);
  return *bytes * 0xD371285717693758L + 0x8186734152378301L;
}

static inline u64 hyper_hash(const void * data, size_t count){
  u64 bias = 0xcbf29ce484222325UL;
  u64 prime1 = 0x100000001b3;
  u64 prime2 = 18446744073709551557UL;
  u64 hash = bias;
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


static inline u32 fnv1a_hash2(const char * data, size_t count){
  return (u32)fnv1a_hash(data, count);
}

u32 hash1(const char * data, size_t count){
  return fnv1a_hash2(data, count);
  //return djb2_hash(data, count);
  //return hyper_hash(data, count);
}


size_t ht_count(hash_table * ht){
  return ht->count;
}

size_t ht_calc_hash(hash_table * ht, void * key){
  i32 hash = ht->hash == NULL ? (i32)hash1(key, ht->key_size) : ht->hash(key, ht->userdata);
  return (size_t)hash;
}
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

void ht_init(hash_table * ht){
  if(ht->keys != NULL)
	return;
  ht->keys = ht->alloc_keys(ht->capacity * ht->key_size);
  ht->elems = ht->alloc_values(ht->capacity * ht->elem_size);
  ht->occupied = ht->alloc_state(ht->capacity * sizeof(ht_state));
  memset(ht->occupied, 0, ht->capacity * sizeof(ht_state));
  if(ht->keys == NULL){
	ERROR("Hashtable keys not set!\n");
  }
}

void ht_set_mem_values(hash_table * ht, void * (* alloc)(size_t s), void (* free)(void * ptr)){
  ht->alloc_values = alloc;
  ht->free_values = free;
}

void ht_set_mem_keys(hash_table * ht, void * (* alloc)(size_t s), void (* free)(void * ptr)){
  ht->alloc_keys = alloc;
  ht->free_keys = free;
}

void * alloc0(size_t s);

void ht_create3(hash_table *ht, size_t capacity, size_t key_size, size_t elem_size){
  memset(ht, 0, sizeof(*ht));
  ht->capacity = capacity == 0 ? 1 : capacity;;
  ht->key_size = key_size;
  ht->elem_size = elem_size;
  //ht->userdata = ht;
  ht_set_alloc(ht, alloc0, free); 
}

hash_table * ht_create2(size_t capacity, size_t key_size, size_t elem_size){
  hash_table * ht = alloc0(sizeof(*ht));
  ht_create3(ht, capacity, key_size, elem_size);
  return ht;
}

hash_table * ht_create(size_t key_size, size_t elem_size){
  return ht_create2(8, key_size, elem_size);
}


void ht_empty(hash_table *ht){
  if(ht->keys != NULL){
    ht->free_keys(ht->keys);
    ht->free_values(ht->elems);
    ht->free_state(ht->occupied);
	ht->keys = NULL;
	ht->elems = NULL;
	ht->occupied = NULL;
  }
}


void ht_free(hash_table *ht){
  var f = ht->free;
  ht->free_keys(ht->keys);
  ht->free_values(ht->elems);
  ht->free_state(ht->occupied);
  memset(ht, 0, sizeof(ht[0]));
  f(ht);
}

void ht_set_capacity(hash_table * ht, size_t buckets){
  if(ht->keys == NULL){
    //just allocate and return.
    ht->capacity = buckets;
    ht_init(ht);
    return;
  }

  // Now we build a new hash table with the added capacity.
  // and replace the original. 
  hash_table * ht2 = ht->alloc(sizeof(*ht));
  ht_create3(ht2, buckets, ht->key_size, ht->elem_size);
  ht2->free_keys = ht->free_keys;
  ht2->free_values = ht->free_values;
  ht2->free_state = ht->free_state;
  ht2->alloc_keys = ht->alloc_keys;
  ht2->alloc_values = ht->alloc_values;
  ht2->alloc_state = ht->alloc_state;
  ht2->alloc = ht->alloc;
  ht2->free = ht->free;
  ht2->hash = ht->hash;
  ht2->compare = ht->compare;
  ht2->userdata = ht->userdata;
  
  // ht_init needs to be called, because
  // tere is not necessarily set any ht->occupied[i].
  ht_init(ht2);
 
  // copy all existingly set values from the previous table to the new one.
  for(u32 i = 0; i < ht->capacity; i++){
    if(ht->occupied[i] == HT_OCCUPIED){
	  ht_set(ht2, ht->keys + i * ht->key_size, ht->elems + i * ht->elem_size);
    }
  }

  // replace the old table with the new and free it.
  SWAP(*ht2, *ht);
  ht->free(ht2);
}

void ht_grow(hash_table * ht){
  ht_set_capacity(ht, ht->capacity * 2);
}

void * memfind0(void * ptr, void * end, size_t size){
  static size_t zeros[32] = {0};
  while(ptr < end){
    if(memcmp(ptr, zeros, size) == 0)
      return ptr;
  }
  return end;
}

static i64 ht_find_free_pre_hashed(const hash_table * ht, size_t hash, const void * key){
  if(ht->keys == NULL)
	return -1;
  size_t key_size = ht->key_size;
  size_t capacity = ht->capacity;
  compare_t compare = ht->compare;
  if(ht->occupied == NULL){

    size_t i1 = hash % capacity;
    void * start = ht->keys + i1 * key_size;
    void * end = ht->keys + capacity * key_size;
    void * unoccupied = memfind0(start, end, key_size);
    if(unoccupied == end){
      // search from beginning
      start = ht->keys;
      end = ht->keys + i1 * key_size;
      unoccupied = memfind0(start, end, key_size);
    }
    if(unoccupied != end)
      return (unoccupied - ht->keys) / key_size;
    return -1;
  }
  
  // find a free slot using linear probing.
  for(size_t _i = 0, i = hash % ht->capacity; _i < capacity; _i++, i++){
    if(i >= capacity) i = 0;
    
    switch(ht->occupied[i]){
    case HT_FREE:
      return i;
    case HT_OCCUPIED:
      {
        void * thiskey = ht->keys + i * key_size;
        if(compare != NULL){
          if(compare(key, thiskey, ht->userdata))
             return i;
        }else if(memcmp(thiskey, key, key_size) == 0){
          return i;
        }
        
      }
    }
  }
  return -1;	 
}

static i64 ht_find_free(const hash_table * ht, const void * key){
  size_t key_size = ht->key_size;
  i32 hash = ht->hash == NULL ? (i32)hash1(key, key_size) : ht->hash(key, ht->userdata);
  return ht_find_free_pre_hashed(ht, hash, key);
}

bool ht_set(hash_table * ht, const void * key, const void * nelem){
  size_t key_size = ht->key_size;
  i32 hash = ht->hash == NULL ? (i32)hash1(key, key_size) : ht->hash(key, ht->userdata);
  return ht_set_precalc(ht, key, nelem, hash);
}

bool ht_set_precalc(hash_table * ht, const void * key, const void * nelem, size_t hash){
  ht_init(ht);

  if(ht->count * 2 >= ht->capacity){
    ht_grow(ht);
  }
  i64 index = ht_find_free_pre_hashed(ht, hash, key);
  size_t elem_size = ht->elem_size;
  size_t key_size = ht->key_size;
  
  if(elem_size > 0)
    memmove(ht->elems + index * elem_size, nelem, elem_size);
  
  if(ht->occupied == NULL){
    if(memcmp(ht->keys + index * key_size, key, key_size) == 0)
      return false;
    
    memmove(ht->keys + index * key_size, key, key_size);
    return true;
  }
  // an unoccupied index was found.
  if(ht->occupied[index] == HT_FREE){
    ht->count += 1;
    memmove(ht->keys + index * key_size, key, key_size);
    ht->occupied[index] = HT_OCCUPIED;
    return true;
  }
  return false;
}

bool ht_get(hash_table * ht, const void *key, void * out_elem){
  if(ht->keys == NULL) return false;
  var index = ht_find_free(ht, key);
  
  if(index == -1)
    return false;
  
  if(ht->occupied != NULL && ht->occupied[index] == HT_FREE)
    return false;
  if(ht->occupied == NULL && memcmp(ht->keys + index * ht->key_size, key, ht->key_size) == 0)
    return false;
  
  var elem_size = ht->elem_size;
  if(out_elem != NULL && elem_size > 0)
    memcpy(out_elem, ht->elems + index * elem_size, elem_size);
  return true;
}

bool ht_get_precalc(hash_table * ht, size_t hashed_key, const void *key, void * out_elem){
  if(ht->keys == NULL) return false;
  i64 index = ht_find_free_pre_hashed(ht, hashed_key, key);
  if(index == -1)
    return false;
  if(ht->occupied != NULL && ht->occupied[index] == HT_FREE )
    return false;
  if(ht->occupied == NULL && memcmp(ht->keys + index * ht->key_size, key, ht->key_size) == 0)
    return false;
  
  size_t elem_size = ht->elem_size;
  if(out_elem != NULL && elem_size > 0)
    memcpy(out_elem, ht->elems + index * elem_size, elem_size);
  return true;
}

void _ht_remove_at(hash_table * ht, size_t index){
  
  ht->count -= 1;
  size_t key_size = ht->key_size;
  size_t elem_size = ht->elem_size;
  size_t capacity = ht->capacity;
  
 start:
  ht->occupied[index] = HT_FREE;
  for(size_t _i = 1, i = index + 1; _i < capacity; _i++, i++){
    // probe to find an element that can be moved back.
    if(i >= capacity) i = 0;
    if(ht->occupied[i] == HT_FREE)
      break;
    
    void * thiskey = ht->keys + i * key_size;
    int hash = ht->hash == NULL ? (i32)hash1(thiskey, key_size) : ht->hash(thiskey, ht->userdata);
    size_t h2 = (size_t)(hash % capacity);
    if(h2 <= index){
      
      ht->occupied[index] = ht->occupied[i];
      memcpy(ht->keys + index * key_size, ht->keys + i * key_size, key_size);
      memcpy(ht->elems + index * elem_size, ht->elems + i * elem_size, elem_size);

      // set index to i and start another iteration.
      index = i;
      goto start;
    } 
  }
}


i64 _ht_remove(hash_table * ht, const void * key){
  i64 index = ht_find_free(ht, key);
  if(ht->occupied[index] == HT_FREE)
    return -1;
  _ht_remove_at(ht, index);
  return index;
}

bool ht_remove(hash_table * ht, const void * key){
  if(ht->keys == NULL) return false;
  i64 index =_ht_remove(ht, key);
  return index != -1;
}

bool ht_remove2(hash_table * ht, const void * key, void * out_key, void * out_elem){
  if(ht->keys == NULL) return false;
  i64 index = ht_find_free(ht, key);
  if(index == -1) return false;
  
  if(out_key)
    memcpy(out_key, ht->keys + index * ht->key_size, ht->key_size); 
  if(out_elem)
    memcpy(out_elem, ht->elems + index * ht->elem_size, ht->elem_size);
  _ht_remove_at(ht, index);
  return true;
}

void ht_clear(hash_table * ht){
  if(ht->keys == NULL) return;
  for(u32 i = 0; i < ht->capacity; i++)
    ht->occupied[i] = HT_FREE;
  
  ht->count = 0;
}

void ht_iterate(hash_table * ht, void (* it)(void * key, void * elem, void * user_data), void * userdata){
  if(ht->keys == NULL) return;
  size_t key_size = ht->key_size;
  size_t elem_size = ht->elem_size;
  for(size_t i = 0; i < ht->capacity; i++){
    if(ht->occupied[i] != HT_OCCUPIED) continue;
    it(ht->keys + i * key_size, ht->elems + i * elem_size, userdata);
  }
}


void ht_iterate2(hash_table * ht, ht_op (* it)(void * key, void * elem, void * user_data), void * userdata){
  if(ht->keys == NULL) return;
  size_t key_size = ht->key_size;
  size_t elem_size = ht->elem_size;
  for(size_t i = 0; i < ht->capacity; i++){
    if(ht->occupied[i] != HT_OCCUPIED) continue;    
    var op = it(ht->keys + i * key_size, ht->elems + i * elem_size, userdata);
    if(op == HT_REMOVE){
      _ht_remove_at(ht, i); 
      i--; // elements might have been moved back.     
    }
  }
}


static i32 string_hash(const char ** key, void * userdata){
  UNUSED(userdata);
  if(*key == NULL) return 0;
  u32 h = hash1(*key, strlen(*key));
  return (i32)h;
}

static bool string_compare(const char ** key1, const char ** key2, void * userdata){
  UNUSED(userdata);
  if(*key2 == *key1) return true;
  if(*key2 == NULL)
    return *key1 == NULL;
  if(*key1 == NULL)
    return false;
  return strcmp(*key1, *key2) == 0;
}

hash_table * ht_create_strkey(size_t elem_size){
  hash_table * ht = ht_create(sizeof(char *), elem_size);
  ht_set_hash(ht, (i32 (*)(const void * a, void * userdata))string_hash);
  ht_set_compare(ht, (bool (*)(const void * a, const void * b, void * userdata))string_compare);
  ht->string_table = true;
  return ht;
}
