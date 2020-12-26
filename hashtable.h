// Hashing

// Hash table 
typedef struct _hash_table hash_table;
// Creats a new hash table.
// This table grows when more than half the slots are filled.
hash_table * ht_create2(size_t init_capacity, size_t key_size, size_t elem_size);
hash_table * ht_create(size_t key_size, size_t elem_size);
hash_table * ht_create_strkey(size_t elem_size);
// Returns a pointer to an element if the key can be found in the table otherwise NULL.
bool ht_get(hash_table * ht, const void * key, void * out_value);

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
