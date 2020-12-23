// Hashing

// Hash table 
typedef struct _hash_table hash_table;
// Creats a new hash table.
// This table grows when more than half the slots are filled.
hash_table * ht_create(u32 init_capacity, u32 key_size, u32 elem_size);

// Returns a pointer to an element if the key can be found in the table otherwise NULL.
bool ht_lookup(hash_table * ht, void * key, void * out_value);

// Inserts an element with a key.
// returns true if a new element is inserted.
bool ht_insert(hash_table * ht, void * key, void * elem);

// Removes an element in the table.
// returns true when an element is removed
// returns false if the element was not found
bool ht_remove(hash_table * ht, void * key);
