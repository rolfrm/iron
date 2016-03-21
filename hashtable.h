// Hashing

i32 jump_consistent_hash(u64 key, i32 num_buckets);
i32 jump_consistent_hash_raw(void * data, size_t length, i32 num_buckets);
i32 jump_consistent_hash_str(char * str, i32 num_buckets);

// aligned hash requiring that datasize is a multiple of sizeof(size_t).
u64 aligned_hash(void * data, size_t datasize);

i64 iron_hash(void * data, u64 len);
void iron_hash_init();
// Hash table //
typedef struct _hash_table hash_table;
// Creats a new hash table. hf must return a 0 <= number < buckets for arbitrary key_data.
hash_table * ht_create(u32 buckets, u32 key_size, u32 elem_size);

// sets the hash functions of the table. hash_fcn must return a value < buckets.
void ht_set_hash_fcn(hash_table * ht,
		     i32 (* hash_fcn)(void * key_data, void * userdata),
		     bool (* key_compare)(void * key_a, void * key_b, void * userdata), 
		     void * userdata);

// Returns a pointer to an element if the key can be found in the table otherwise NULL.
void * ht_lookup(hash_table * ht, void * key);

// Inserts an element with a key.
void ht_insert(hash_table * ht, void * key, void * elem);

// Removes an element in the table.
void ht_remove(hash_table * ht, void * key);

typedef struct _dod_hash_table dod_hash_table;
dod_hash_table * dod_ht_create(size_t n_cells, size_t key_size, size_t elem_size);
void dod_ht_insert(dod_hash_table * hashtable, void * keys, void * values, size_t cnt);
void dod_ht_remove(dod_hash_table * hashtable, void * keys, size_t cnt);
void dod_ht_lookup(dod_hash_table * hashtable, void * keys, void * out_values, int cnt);
void dod_ht_clear(dod_hash_table * hashtable);
