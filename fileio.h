void * get_format_out();
void format(char * fmt, ...);
void with_format_out(void * file, void (* fcn)());
void dump_buffer_to_file(void * buffer,size_t s, char * filepath);
