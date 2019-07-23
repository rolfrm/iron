
typedef struct _gl_window gl_window;

typedef enum{
  IRON_GL_BACKEND_GLFW,
  IRON_GL_BACKEND_X11
}IRON_GL_BACKEND;

extern IRON_GL_BACKEND iron_gl_backend;

typedef struct{
  void (* poll_events)();
  void (* init)();
  void (* deinit)();
  void * (* create_window)(int width, int height, const char * title);
  void (* destroy_window)(void * window);
  void (* make_current)(void * window);
  void (* swap_buffers)(void * window);
  void (* get_window_size) (void * window, int * w, int *h);
  void (* get_cursor_position)(void * window, int * x, int * y);
  bool (* get_button_state)(void * window, int button);
  bool (* get_key_state) (void * window, int key);
}gl_backend;
extern gl_backend * current_backend;

void gl_set_backend(IRON_GL_BACKEND backend);

gl_window * gl_window_open(int width, int height);

void gl_window_swap(gl_window *);
void gl_window_get_size(gl_window * win, int * width, int *height);

void gl_window_make_current(gl_window * win);
void gl_window_destroy(gl_window **);
void gl_window_poll_events();
bool gl_window_get_btn_state(gl_window * win, int btn);
bool gl_window_get_key_state(gl_window * win, int key);
typedef enum{
  EVT_MOUSE_MOVE,
  EVT_MOUSE_LEAVE,
  EVT_MOUSE_ENTER,
  EVT_MOUSE_BTN_DOWN,
  EVT_MOUSE_BTN_UP,
  EVT_MOUSE_SCROLL,
  EVT_KEY_DOWN,
  EVT_KEY_UP,
  EVT_WINDOW_CLOSE,
  EVT_WINDOW_MINIMIZE,
  EVT_WINDOW_MAXIMIZE,
  EVT_WINDOW_RESTORE,
  EVT_WINDOW_CUSTOM,
  EVT_WINDOW_REFRESH
}gl_event_known_event_types;

typedef struct _gl_window_event{
  u64 timestamp;
  u64 type;
  gl_window * win;
  u8 data[32];
}gl_window_event;

size_t gl_get_events(gl_window_event * event_buffer, size_t max_read);

typedef struct{
  u64 timestamp;
  u64 type;
  gl_window * win;
  
  int x, y;
}evt_mouse_move;

typedef struct{
  u64 timestamp;
  u64 type;
  gl_window * win;
  
  int button;
} evt_mouse_btn;

typedef struct{
  u64 timestamp;
  u64 type;
  gl_window * win;
  
  double scroll_x, scroll_y;
}evt_mouse_scroll;

typedef struct{
  u64 timestamp;
  u64 type;
  gl_window * win;
  bool ischar;
  int key;
  char codept;
}evt_key;


void get_mouse_position(gl_window * win, int * x, int * y);

enum{
  KEY_UP = 265,
  KEY_DOWN = 264,
  KEY_LEFT = 263,
  KEY_RIGHT = 262,
  KEY_ENTER = 257,
  KEY_ESC =  256,
  KEY_CTRL = 341,
  KEY_SHIFT = 340,
  KEY_F1 = 290,
  KEY_F2 = 291,
  KEY_F3 = 292,
  KEY_F4 = 293,
  KEY_F5 = 294,
  KEY_F6 = 295,
  KEY_F7 = 296,
  KEY_F8 = 297,
  KEY_F9 = 298,
  KEY_F10 = 299,
  KEY_F11 = 300,
  KEY_F12 = 301,
};

void register_evt(void * win, void * _evt, gl_event_known_event_types type);
// images
typedef struct _image_source image_source;

typedef struct{
  image_source * source;
  int width, height, channels;
}image;

void * image_data(image * image);
image image_from_file(const char * path);
image image_from_data(void * data, int len);
image image_new(int width, int height, int channels);
void image_delete(image * image);

// textures
typedef struct _texture_handle texture_handle;
typedef struct {
  texture_handle * handle;
  int width, height;
}texture;

typedef enum{
  TEXTURE_INTERPOLATION_NEAREST,
  TEXTURE_INTERPOLATION_BILINEAR,
  
}TEXTURE_INTERPOLATION;

texture texture_from_image(image * image);
texture texture_from_image2(image * image, TEXTURE_INTERPOLATION interp);

// blitting
typedef enum{
  BLIT_MODE_PIXEL,
  BLIT_MODE_UNIT

}BLIT_MODE;

void blit_begin(BLIT_MODE blit_mode);
void blit_end();
void blit_translate(float x, float y);
void blit_scale(float x, float y);
void blit(float x, float y, texture * texture);
void blit_rectangle(float x, float y, float w, float h, float r, float g, float b, float a);

mat3 blit_get_view_transform();

