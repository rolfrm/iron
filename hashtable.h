// Hashing

// Hash table 
typedef bool (* compare_t ) (const void * k1, const void * k2, void * userdata);

typedef enum {
	      HT_FREE = 0,
	      HT_OCCUPIED = 1
}ht_state;

typedef struct _hash_table hash_table;
struct _hash_table{
  // this is allowed to be null
  i32 (* hash )(const void * key_data, void * userdata);
  // this is allowed to be null
  void * userdata;
  // this is allowed to be null
  compare_t compare;

  void * (* alloc_keys)(size_t size);
  void * (* alloc_values)(size_t size);
  void * (* alloc_state)(size_t size);
  void (* free_keys)(void * ptr);
  void (* free_values)(void * ptr);
  void (* free_state)(void * ptr);
  
  
  void * keys;
  void * elems;
  // this may be null if NULL means empty key
  ht_state * occupied;

  size_t capacity;
  size_t count;
  
  size_t key_size;
  size_t elem_size;
  bool string_table;
};

// Creats a new hash table.
// This table grows when more than half the slots are filled.
hash_table * ht_create2(size_t init_capacity, size_t key_size, size_t elem_size);
hash_table * ht_create(size_t key_size, size_t elem_size);
hash_table * ht_create_strkey(size_t elem_size);
// Returns a pointer to an element if the key can be found in the table otherwise NULL.
bool ht_get(hash_table * ht, const void * key, void * out_value);
size_t ht_calc_hash(hash_table * ht, void * key);
bool ht_get_precalc(hash_table * ht, size_t hashed_key, const void *key, void * out_elem);
bool ht_set_precalc(hash_table * ht, const void *key, const void * out_elem, size_t hash);

// Inserts an element with a key.
// returns true if a new key is inserted.
// returns false if the key existed. In this case the elem is still overwritten.
bool ht_set(hash_table * ht, const void * key, const void * elem);

// Removes an element in the table.
// returns true when an element is removed
// returns false if the element was not found
bool ht_remove(hash_table * ht, const void * key);

// Removes an element from the table.
// out_key and out_elem can be set to NULL when not needed.
// the removed key and value will be writen to those locations.
bool ht_remove2(hash_table * ht, const void * key, void * out_key, void * out_elem);

// Clears the entire table without freeing the data.
void ht_clear(hash_table * ht);

// sets the hashing function of the hash table
void ht_set_hash(hash_table * ht, i32 (* hash)(const void * key,  void * userdata));

// sets the compare function of the hash table
void ht_set_compare(hash_table * ht, bool (* compare)(const void * k1, const void * k2, void * userdata));

// iterate hash table
void ht_iterate(hash_table * ht, void (* it)(void * key, void * elem, void * user_data), void * userdata);

