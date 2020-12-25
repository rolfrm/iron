// Hashing

// Hash table 
typedef struct _hash_table hash_table;
// Creats a new hash table.
// This table grows when more than half the slots are filled.
hash_table * ht_create2(size_t init_capacity, size_t key_size, size_t elem_size);
hash_table * ht_create(size_t key_size, size_t elem_size);

// Returns a pointer to an element if the key can be found in the table otherwise NULL.
bool ht_lookup(hash_table * ht, const void * key, void * out_value);

// Inserts an element with a key.
// returns true if a new element is inserted.
bool ht_insert(hash_table * ht, const void * key, const void * elem);

// Removes an element in the table.
// returns true when an element is removed
// returns false if the element was not found
bool ht_remove(hash_table * ht, const void * key);

// Clears the entire table without freeing the data.
void ht_clear(hash_table * ht);

// sets the hashing function of the hash table
void ht_set_hash(hash_table * ht, i32 (* hash)(const void * key,  void * userdata));

// sets the compare function of the hash table
void ht_set_compare(hash_table * ht, bool (* compare)(const void * k1, const void * k2, void * userdata));
