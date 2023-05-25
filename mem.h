// rquires stdbool
// custom allocator functionality

typedef struct _allocator allocator;

struct _allocator{
  void * (* alloc)(allocator * self, size_t size);
  void (* dealloc) (allocator * self, void * ptr);
  void *(* ralloc)(allocator * self, void * ptr, size_t size);
  void * user_data;
};

void with_allocator(allocator * alc, void (* cb)(void));
allocator * iron_get_allocator(void);
void iron_set_allocator(allocator * alc);

void * alloc(size_t);
void dealloc(void * ptr);
void * ralloc(void * ptr, size_t size);
void * alloc0(size_t);

void * iron_clone(const void * src, size_t s);

#define RALLOC(PTR,NEW_SIZE) PTR = ralloc(PTR, (NEW_SIZE) * sizeof(PTR[0]))

#define IRON_CLONE(object) ({auto data = object; iron_clone(&data, sizeof(data));})

allocator * block_allocator_make(void);
void block_allocator_release(allocator * block_allocator);

allocator * trace_allocator_make(void);
size_t trace_allocator_used_mem(void);
size_t trace_allocator_allocated_pointers(allocator * trace_allocator);
void trace_allocator_release(allocator * trace_allocator);

char * fmtstr(const char * fmt, ...);

// buffered fmt for small things.
// only one value can be used at a time.
const char * quickfmt(const char * fmt, ...);

#define new(type) alloc0(sizeof(type))

bool string_startswith(const char * target, const char * test);
// returns the things following substring in target, unless it does not contain it.
char * string_skip(char * target, const char * substring);
char * string_skip_all(char * target, const char * substring);
char * string_join(int cnt, const char * separator, char ** strings);
void replace_inplace(char * out_buffer, const char * pattern, const char * insert);
char ** string_split(char * str, const char * pattern, int * out_cnt);
 
