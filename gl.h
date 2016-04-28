
typedef struct _gl_window gl_window;

gl_window * gl_window_open(int width, int height);

void gl_window_swap(gl_window *);

void gl_window_make_current(gl_window * win);
void gl_window_destroy(gl_window **);

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
  EVT_WINDOW_CUSTOM
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
}evt_mouse_btn;

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
