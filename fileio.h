void * get_format_out();
void format(char * fmt, ...);
void with_format_out(void * file, void (* fcn)());
void write_buffer_to_file(void * buffer,size_t s, char * filepath);
void append_buffer_to_file(void * buffer, size_t s, char * filepath);
