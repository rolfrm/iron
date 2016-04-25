
typedef struct _gl_window gl_window;

gl_window * gl_window_open(int width, int height);

void gl_window_swap(gl_window *);

void gl_window_make_current(gl_window * win);
void gl_window_destroy(gl_window **);
void gl_window_terminate();
