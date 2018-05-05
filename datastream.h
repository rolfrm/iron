
typedef struct {
  const char * name;
  const char * parent_stream_name;

  // for internal use only.
  void * internal;
}data_stream;

typedef struct {
  void (* process)(const data_stream * stream, const void * data, size_t length, void * userdata);
  void * userdata;
}data_stream_listener;

void data_stream_message(const data_stream * stream, const char * msg, ...);
void data_stream_data(const data_stream * stream, const void * data, size_t length);

void data_stream_listen(data_stream_listener * listener, data_stream * stream);
void data_stream_unlisten(data_stream_listener * listener, data_stream * stream);
void data_stream_listen_all(data_stream_listener * listener);
void data_stream_unlisten_all(data_stream_listener * listener);

#define dmsg(stream, msg, ...) data_stream_message(&stream, msg, ##__VA_ARGS__);
#define dlog(stream, data, length) data_stream_data(&stream, data, length);

