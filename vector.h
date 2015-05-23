// Vector //
typedef struct _vector vector;
vector * vector_create(size_t elem_size);
void * vector_lookup(vector * vec,size_t idx);
void * vector_lookup_unsafe(vector * vec,size_t idx);
void * vector_push(vector * vec, void * buffer, size_t count);
void * vector_pop(vector * vec, size_t count);
size_t vector_size(vector * vec);
