#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "types.h"
#include "hashtable.h"


//A Fast, Minimal Memory, Consistent Hash Algorithm
// John Lamping, Eric Veach
i32 jump_consistent_hash(u64 key, i32 num_buckets){
  i64 b = -1, j = 0;
  while(j < num_buckets){
    b = j;      
    key = key * 2862933555777941757ULL + 1;
    double v1 = 1LL << 31;
    double v2 = (key >> 33) + 1;
    j = (b + 1) * v1 / (v2 );
  }
  return b;
}

i32 jump_consistent_hash_raw(void * data, size_t length, i32 num_buckets){
  i32 hash = 0;
  u64 * keys = (u64 *) data;
  while(length > sizeof(u64))
    {
      hash ^= jump_consistent_hash(*keys,num_buckets);
      keys++;
      length -=8;
    }
  u64 val = 0;
  memcpy(&val,keys,length);
  hash ^= jump_consistent_hash(val,num_buckets);
  return hash;  
}
 
i32 jump_consistent_hash_str(char * str, i32 num_buckets){
  return jump_consistent_hash_raw(str,strlen(str),num_buckets);
}



struct _hash_table{
  i32 (* hf )(void * key_data, void * userdata);
  void * userdata;
  bool (* compare ) (void * k1, void * k2, void * userdata);
  //an array of arrays
  void ** key_buckets;
  void ** elem_buckets;
  //the number of elems in each bucket
  u32 * bucket_size;
  u32 buckets;
  u32 key_size;
  u32 elem_size;
};

i32 default_hash(void * keydata, hash_table * ht){
  return jump_consistent_hash_raw(keydata, ht->key_size, ht->buckets);
}

bool default_compare(void * key_a, void * key_b, hash_table * ht){
  return 0 == memcmp(key_a, key_b, ht->key_size);
}

// a simple but not super efficient hash table implementation.
hash_table * ht_create(u32 buckets, u32 key_size, u32 elem_size){
  hash_table * ht = malloc(sizeof(hash_table));
  memset(ht,0,sizeof(hash_table));
  ht->buckets = buckets;
  ht->key_size = key_size;
  ht->elem_size = elem_size;
  ht->hf = (i32 (*)(void* ,void*))default_hash;
  ht->compare = (bool (*)(void*, void*, void*))default_compare;
  ht->userdata = ht;
  ht->key_buckets = malloc(ht->buckets * sizeof(void *));
  ht->elem_buckets = malloc(ht->buckets * sizeof(void *));
  ht->bucket_size = malloc(ht->buckets * sizeof(u32));
  memset(ht->key_buckets,0,ht->buckets * sizeof(void *));
  memset(ht->elem_buckets,0,ht->buckets * sizeof(void *));
  memset(ht->bucket_size,0,ht->buckets * sizeof(u32));
  return ht;
}

void ht_set_hash_fcn(hash_table * ht,
		     i32 (* hf)(void * key_data, void * userdata),
		     bool (* key_compare)(void * key_a, void * key_b, void * userdata), 
		     void * userdata){
  ht->hf = hf;
  ht->compare = key_compare;
  ht->userdata = userdata;
}

void * ht_lookup(hash_table * ht, void * key){
  i32 hash = ht->hf(key,ht->userdata);
  u32 elem_size = ht->elem_size;
  u32 key_size = ht->key_size;
  void * keys = ht->key_buckets[hash];
  void * elem = ht->elem_buckets[hash];
  int aloc = ht->bucket_size[hash];
  for(int i = 0; i < aloc;i++){
    if(ht->compare(keys + key_size * i, key, ht->userdata)){
      return elem + i * elem_size;
    }
  }
  return NULL;
}
#include <stdio.h>
void ht_insert(hash_table * ht, void * key, void * nelem){
  i32 hash = ht->hf(key,ht->userdata);
  u32 elem_size = ht->elem_size;
  u32 key_size = ht->key_size;
  void * keys = ht->key_buckets[hash];
  void * elem = ht->elem_buckets[hash];
  int aloc = ht->bucket_size[hash];

  for(int i = 0; i < aloc; i++){
    if(ht->compare(keys + key_size * i,key,ht->userdata)){
      memcpy(elem + elem_size * i, nelem,elem_size);
      return;
    }
  }
  //alloc new room
  keys = realloc(keys, (aloc + 1) * key_size);
  elem = realloc(elem, (aloc + 1) * elem_size);
  memcpy(elem + elem_size * aloc, nelem,elem_size);
  memcpy(keys + key_size * aloc, key,key_size);
  ht->bucket_size[hash] = aloc + 1;
  ht->key_buckets[hash] = keys;
  ht->elem_buckets[hash] = elem;
}

void ht_remove(hash_table * ht, void * key){
  i32 hash = ht->hf(key,ht->userdata);
  u32 elem_size = ht->elem_size;
  u32 key_size = ht->key_size;
  void * keys = ht->key_buckets[hash];
  void * elem = ht->elem_buckets[hash];
  int aloc = ht->bucket_size[hash];  
  for(int i = 0; i < aloc; i++){
    if(ht->compare(keys + key_size * i,key,ht->userdata)){
      memmove(elem + elem_size * i, elem + elem_size * (i + 1), elem_size * MAX(0, aloc - i - 1));
      memmove(keys + key_size * i, keys + key_size * (i + 1), key_size * MAX(0, aloc - i - 1));
      keys = realloc(keys, (aloc - 1) * key_size);
      elem = realloc(elem, (aloc - 1) * elem_size);
      aloc -= 1;
      
      ht->key_buckets[hash] = keys;
      ht->elem_buckets[hash] = elem;
      ht->bucket_size[hash] = aloc;
      return;
    }
  }
}

/*typedef struct {
  u64 * keys;
  u64 * values;
  u64 cnt;
}dod_hash_cell;

struct _dod_hash_table{
  dod_hash_cell * cells;
  size_t n_cells;
  size_t elem_size;
  size_t key_size;
};
const size_t hash_size = sizeof(i32);
i32 calc_hash(void * item, size_t item_size){
  UNUSED(item);
  UNUSED(item_size);
  return 0;
}
static void calc_hashes(void * items_to_hash, void * out_buffer, size_t cnt, size_t key_size){
  for(int i = 0; i < cnt; i++){
    void * item_ptr = items_to_hash + key_size * i;
    i32 * out_ptr = out_buffer + i * hash_size;
    *out_ptr = calc_hash(item_ptr, key_size);
  }
}

dod_hash_table * dod_ht_create(size_t n_cells, size_t key_size, size_t elem_size){
  dod_hash_table t;
  t.cells = alloc0(n_cells * sizeof(dod_hash_cell));
  t.n_cells = n_cells;
  t.elem_size = elem_size;
  t.key_size = key_size;
  return clone(&t, sizeof(dod_hash_table));
}

void dod_ht_insert(dod_hash_table * t, void * k, void * v, size_t cnt){
  __thread__ static void * key_buffer = NULL;
  __thread__ static size_t key_buffer_length = 0;
  if(key_buffer_length < cnt){
    key_buffer = realloc(key_buffer, cnt * t->key_size);
    key_buffer_length = cnt;
  }
  calc_hashes(k, key_buffer, cnt, t->key_size);
  // now i need to sort 'k' 'v' and key_buffer by key_buffer;
  
}
*/
