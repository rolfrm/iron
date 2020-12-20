
typedef struct _gl_window gl_window;

typedef enum{
  IRON_GL_BACKEND_GLFW,
  IRON_GL_BACKEND_X11
}IRON_GL_BACKEND;

typedef enum{
  IRON_CURSOR_NORMAL,
  IRON_CURSOR_CROSSHAIR

}iron_cursor_type;
extern IRON_GL_BACKEND iron_gl_backend;
extern bool iron_gl_debug;
typedef struct{
  void (* poll_events)();
  void (* init)();
  void (* deinit)();
  void * (* create_window)(int width, int height, const char * title);
  void (* destroy_window)(void * window);
  void (* make_current)(void * window);
  void (* swap_buffers)(void * window);
  void (* get_window_size) (void * window, int * w, int *h);
  void (* set_window_size) (void * window, int w, int h);
  void (* get_window_position)(void * window, int * x, int * y);
  void (* set_window_position)(void * window, int x, int y);
  void (* get_cursor_position)(void * window, int * x, int * y);
  bool (* get_button_state)(void * window, int button);
  bool (* get_key_state) (void * window, int key);
  void (* set_cursor_type)( void * window, iron_cursor_type ctype);
  void (* show_cursor)( void * window, bool show);
  const char * (* get_clipboard)(void * window);
  void (* set_window_title)(void * window, const char * new_title);
}gl_backend;
extern gl_backend * current_backend;

void gl_set_backend(IRON_GL_BACKEND backend);

gl_window * gl_window_open(int width, int height);

void gl_window_swap(gl_window *);
void gl_window_get_size(gl_window * win, int * width, int *height);
void gl_window_set_size(gl_window * win, int width, int height);
void gl_window_set_position(gl_window * win, int x, int y);
void gl_window_get_position(gl_window * win, int *x, int * y);
void gl_window_make_current(gl_window * win);
void gl_window_destroy(gl_window **);
void gl_window_poll_events();
bool gl_window_get_btn_state(gl_window * win, int btn);
bool gl_window_get_key_state(gl_window * win, int key);
const char * gl_window_get_clipboard(gl_window * win);
void gl_window_set_title(gl_window * win, const char * title);
void gl_terminate();
typedef enum{
  EVT_MOUSE_MOVE,
  EVT_MOUSE_LEAVE,
  EVT_MOUSE_ENTER,
  EVT_MOUSE_BTN_DOWN,
  EVT_MOUSE_BTN_UP,
  EVT_MOUSE_SCROLL,
  EVT_KEY_DOWN,
  EVT_KEY_UP,
  EVT_KEY_REPEAT,
  EVT_WINDOW_CLOSE,
  EVT_WINDOW_MINIMIZE,
  EVT_WINDOW_MAXIMIZE,
  EVT_WINDOW_RESTORE,
  EVT_WINDOW_RESIZE,
  EVT_WINDOW_CUSTOM,
  EVT_WINDOW_REFRESH,
  EVT_WINDOW_MOVE
}gl_event_known_event_types;

void gl_window_set_cursor_type(gl_window * win, iron_cursor_type type);
void gl_window_show_cursor(gl_window * win, bool);
typedef struct _gl_window_event{
  u64 timestamp;
  u64 type;
  gl_window * win;
  union{
    struct{
      int x, y;
    }mouse_move;
    struct{
      double x, y;
    }mouse_scroll;
    
    struct{
      int button;
    }mouse_btn;
    struct{
      bool ischar;
      int key;
      char codept;
      int scancode;
    }key;

    struct{
      int width, height;
    }window_size_change;

    struct{
      int x, y;
    }window_position_change;
  };
}gl_window_event;

size_t gl_get_events(gl_window_event * event_buffer, size_t max_read);


u32 gl_shader_compile(const char * vertex_src, const char * fragment_src);
u32 gl_shader_compile2(const char * vertex_src, int vertex_src_len, const char * fragment_src, int fragment_len);
u32 gl_compile_compute_shader(const char * compute_shader, int compute_shader_len);


void get_mouse_position(gl_window * win, int * x, int * y);

enum{
  KEY_UP = 265,
  KEY_DOWN = 264,
  KEY_LEFT = 263,
  KEY_RIGHT = 262,
  KEY_ESC =  256,
  KEY_ENTER = 257,
  KEY_BACKSPACE = 259,

  KEY_SHIFT = 340,
  KEY_CTRL = 341,
  KEY_ALT = 342,
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
  KEY_SPACE = 32,
  KEY_0 = 48,
  KEY_1 = 49,
  KEY_2 = 50,
  KEY_3 = 51,
  KEY_4 = 52,
  KEY_5 = 53,
  KEY_6 = 54,
  KEY_7 = 55,
  KEY_8 = 56,
  KEY_9 = 57,  
  KEY_A = 65,
  KEY_B = 66,
  KEY_C = 67,
  KEY_D = 68,
  KEY_E = 69,
  KEY_F = 70,
  KEY_G = 71,
  KEY_H = 72,
  KEY_I = 73,
  KEY_J = 74,
  KEY_K = 75,
  KEY_L = 76,
  KEY_M = 77,
  KEY_N = 78,
  KEY_O = 79,
  KEY_P = 80,
  KEY_Q = 81,
  KEY_R = 82,
  KEY_S = 83,
  KEY_T = 84,
  KEY_U = 85,
  KEY_V = 86,
  KEY_W = 87,
  KEY_X = 88,
  KEY_Y = 89,
  KEY_Z = 90
};

void register_evt(void * win, gl_window_event * _evt, gl_event_known_event_types type);

// images

typedef enum{
  IMAGE_MODE_NONE = 0,
  IMAGE_MODE_GRAY_AS_ALPHA = 1,
  IMAGE_MODE_F32 = 2
  
}image_mode;

typedef struct _image_source image_source;

typedef struct{
  image_source * source;
  int width, height, channels;
  image_mode mode;
}image;

void * image_data(image * image);
image image_from_file(const char * path);
image image_from_data(void * data, int len);
image image_from_bitmap(void * bitmap, int width, int height, int channels);
image image_new(int width, int height, int channels);
image image_new2(int width, int height, int channels, image_mode mode);
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
  TEXTURE_INTERPOLATION_LINEAR,
  
}TEXTURE_INTERPOLATION;

typedef enum{
  TEXTURE_BIND_WRITE = 1,
  TEXTURE_BIND_READ = 2,
  TEXTURE_BIND_READ_WRITE = 3,
  
}texture_bind_options;

texture texture_from_image(image * image);
texture texture_from_image2(image * image, TEXTURE_INTERPOLATION interp);
texture texture_from_image3(image * image, TEXTURE_INTERPOLATION sub_interp, TEXTURE_INTERPOLATION super_interp);
void texture_load_image(texture * texture, image * image);
void texture_to_image(texture * tex, image * image);
void gl_texture_bind(texture tex);
void gl_texture_image_bind(texture tex, int channel, texture_bind_options options);
u32 gl_texture_handle(texture tex);
// font
typedef struct _font font;
font * blit_load_font_from_buffer(void * data, float font_size);
font * blit_load_font_file(const char * fontfile, float font_size);
void blit_set_current_font(font * fnt);
vec2 blit_measure_text(const char * text);

// blitting
typedef enum{
  BLIT_MODE_PIXEL = 1,
  BLIT_MODE_UNIT = 2,
  BLIT_MODE_SCREEN_BIT = 4,
  BLIT_MODE_PIXEL_SCREEN = 1|BLIT_MODE_SCREEN_BIT,
}BLIT_MODE;

BLIT_MODE blit_mode_get();
void blit_clear();
void blit_begin(BLIT_MODE blit_mode);
void blit_end();
void blit_translate(float x, float y);
void blit_scale(float x, float y);
void blit(float x, float y, texture * texture);
void blit2(texture * texture);
void blit_rectangle(float x, float y, float w, float h, float r, float g, float b, float a);
void blit_rectangle2(float r, float g, float b, float a);
void blit_text(const char * text);

void blit_uv_matrix(mat3 uv);
void blit_push();
void blit_pop();
void blit_bind_texture(texture * tex);
void blit_quad();
void blit_color(f32 r, f32 g, f32 b ,f32 a);
mat3 blit_get_view_transform();
vec2 blit_translate_point(vec2 p);
typedef struct{
  u32 id;
  image_mode mode;
  u32 channels;
  int width, height;

  // -- INTERNALS --
  texture_handle * texture;
}blit_framebuffer;

void blit_create_framebuffer(blit_framebuffer * buf);
void blit_use_framebuffer(blit_framebuffer * buf);
void blit_unuse_framebuffer(blit_framebuffer * buf);
void blit_blit_framebuffer(blit_framebuffer * buf);
void blit_delete_framebuffer(blit_framebuffer * buf);
texture blit_framebuffer_as_texture(blit_framebuffer * buf);

typedef struct _blit3d_context blit3d_context;
blit3d_context * blit3d_context_new();
void blit3d_context_load(blit3d_context * ctx);

typedef enum{
  VERTEX_BUFFER_ARRAY = 1,
  VERTEX_BUFFER_ELEMENTS = 2
}vertex_buffer_type;

typedef struct _blit3d_polygon blit3d_polygon;
typedef blit3d_polygon vertex_buffer;
blit3d_polygon * blit3d_polygon_new();
void blit3d_polygon_load_data(blit3d_polygon * polygon, void * data, size_t size);
void blit3d_polygon_destroy(blit3d_polygon ** polygon);
  
void blit3d_polygon_configure(blit3d_polygon * polygon, int dimensions);

void blit3d_view(blit3d_context * ctx, mat4 viewmatrix);


void blit3d_color(blit3d_context * ctx, vec4 color);
void blit3d_bind_texture(blit3d_context * ctx, texture * tex);
void blit3d_polygon_blit(blit3d_context * ctx, blit3d_polygon * polygon);
void blit3d_polygon_blit2(blit3d_context * ctx, vertex_buffer ** polygons, u32 count);
