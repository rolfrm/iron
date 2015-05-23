// custom allocator functionality

typedef struct{
  void * (* alloc)(size_t size);
  void (* dealloc) (void * ptr);
  void *(* ralloc)(void * ptr, size_t size);
  void * user_data;
}allocator;

void with_allocator(allocator * alc, void (* cb)());

void * alloc(size_t);
void dealloc(void * ptr);
void * ralloc(void * ptr, size_t size);

allocator * block_allocator_make();
void block_allocator_release(allocator * block_allocator);
