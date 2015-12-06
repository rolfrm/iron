void * get_format_out();
void format(const char * fmt, ...);
void with_format_out(void * file, void (* fcn)());
void push_format_out(void * file);
void pop_format_out();
void write_buffer_to_file(const void * buffer,size_t s, const char * filepath);
void append_buffer_to_file(const void * buffer, size_t s, const char * filepath);
char * read_stream_to_string(void * file);
char * read_file_to_string(const char * filepath);
void * read_file_to_buffer(const char * filepath, size_t * out_size);
void * read_file_to_buffer(const char * filepath, size_t * size);

int enter_dir_of(const char * path);
int get_filename(char * buffer, const char * filepath);
