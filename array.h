
void sort_indexed(i64 * ids, u64 count, u64 * out_indexes);
u64 count_uniques_sorted(i64 * items, u64 count);
void get_uniques(i64 * ids, u64 count, i64 * out_uniques);
bool are_sorted(i64 * arr, u64 count);

typedef bool (* selector)(const void *);
// Counts the items of data where selector returns true.
u64 count(void * data, size_t num, size_t element_size, selector selector_fcn);


void apply_arrayi(int * data, int cnt, int (* fcn)(int));
void apply_arrayii(int * data, int cnt, void (* fcn) (int, int));
void apply_arrayd(double * data, int cnt, double (* fcn) (double));
void apply_arraydi(double * data, int cnt, void (* fcn) (double,int));

// Sums the values.
i64 sum64(i64 * values, u64 count);

// List //
// adds an element to the array.
// not very efficient, but easy.
// requires dst to be allocated with malloc or NULL.
// consider calling it stupid_list_add
void list_add(void ** dst, size_t * cnt, void * src, size_t item_size);

void list_clean(void ** lst);

// Returns true if all chars in str are whitespace.
bool all_whitespace(char * str);
