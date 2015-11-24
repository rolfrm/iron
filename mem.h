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
void * alloc0(size_t);

// slower than expected. reallocs nearest power of 2 size. Making int practical for vectors.
//void * ralloc2(void * ptr, size_t size);

void * iron_clone(void * src, size_t s);

allocator * block_allocator_make();
void block_allocator_release(allocator * block_allocator);

allocator * trace_allocator_make();
size_t trace_allocator_used_mem();
size_t trace_allocator_allocated_pointers(allocator * trace_allocator);
void trace_allocator_release(allocator * trace_allocator);

char * fmtstr(char * fmt, ...);
char * vfmtstr(char * fmt, va_list args);
#define new(type) alloc0(sizeof(type))

//test

