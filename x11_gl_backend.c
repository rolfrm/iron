#include "full.h"
#include "gl.h"
#include <X11/Xresource.h>
#include<GL/glx.h>
#include <GLFW/glfw3.h>

static Display * display;
static Window root;
static XVisualInfo * vi;
static Colormap cmap;
static XSetWindowAttributes swa;

int x11_to_glfw_key(int keySym){
  switch (keySym)
    {
    case XK_Escape:         return GLFW_KEY_ESCAPE;
    case XK_Tab:            return GLFW_KEY_TAB;
    case XK_Shift_L:        return GLFW_KEY_LEFT_SHIFT;
    case XK_Shift_R:        return GLFW_KEY_RIGHT_SHIFT;
    case XK_Control_L:      return GLFW_KEY_LEFT_CONTROL;
    case XK_Control_R:      return GLFW_KEY_RIGHT_CONTROL;
    case XK_Meta_L:
    case XK_Alt_L:          return GLFW_KEY_LEFT_ALT;
    case XK_Mode_switch: // Mapped to Alt_R on many keyboards
    case XK_ISO_Level3_Shift: // AltGr on at least some machines
    case XK_Meta_R:
    case XK_Alt_R:          return GLFW_KEY_RIGHT_ALT;
    case XK_Super_L:        return GLFW_KEY_LEFT_SUPER;
    case XK_Super_R:        return GLFW_KEY_RIGHT_SUPER;
    case XK_Menu:           return GLFW_KEY_MENU;
    case XK_Num_Lock:       return GLFW_KEY_NUM_LOCK;
    case XK_Caps_Lock:      return GLFW_KEY_CAPS_LOCK;
    case XK_Print:          return GLFW_KEY_PRINT_SCREEN;
    case XK_Scroll_Lock:    return GLFW_KEY_SCROLL_LOCK;
    case XK_Pause:          return GLFW_KEY_PAUSE;
    case XK_Delete:         return GLFW_KEY_DELETE;
    case XK_BackSpace:      return GLFW_KEY_BACKSPACE;
    case XK_Return:         return GLFW_KEY_ENTER;
    case XK_Home:           return GLFW_KEY_HOME;
    case XK_End:            return GLFW_KEY_END;
    case XK_Page_Up:        return GLFW_KEY_PAGE_UP;
    case XK_Page_Down:      return GLFW_KEY_PAGE_DOWN;
    case XK_Insert:         return GLFW_KEY_INSERT;
    case XK_Left:           return GLFW_KEY_LEFT;
    case XK_Right:          return GLFW_KEY_RIGHT;
    case XK_Down:           return GLFW_KEY_DOWN;
    case XK_Up:             return GLFW_KEY_UP;
    case XK_F1:             return GLFW_KEY_F1;
    case XK_F2:             return GLFW_KEY_F2;
    case XK_F3:             return GLFW_KEY_F3;
    case XK_F4:             return GLFW_KEY_F4;
    case XK_F5:             return GLFW_KEY_F5;
    case XK_F6:             return GLFW_KEY_F6;
    case XK_F7:             return GLFW_KEY_F7;
    case XK_F8:             return GLFW_KEY_F8;
    case XK_F9:             return GLFW_KEY_F9;
    case XK_F10:            return GLFW_KEY_F10;
    case XK_F11:            return GLFW_KEY_F11;
    case XK_F12:            return GLFW_KEY_F12;
    case XK_F13:            return GLFW_KEY_F13;
    case XK_F14:            return GLFW_KEY_F14;
    case XK_F15:            return GLFW_KEY_F15;
    case XK_F16:            return GLFW_KEY_F16;
    case XK_F17:            return GLFW_KEY_F17;
    case XK_F18:            return GLFW_KEY_F18;
    case XK_F19:            return GLFW_KEY_F19;
    case XK_F20:            return GLFW_KEY_F20;
    case XK_F21:            return GLFW_KEY_F21;
    case XK_F22:            return GLFW_KEY_F22;
    case XK_F23:            return GLFW_KEY_F23;
    case XK_F24:            return GLFW_KEY_F24;
    case XK_F25:            return GLFW_KEY_F25;

      // Numeric keypad
    case XK_KP_Divide:      return GLFW_KEY_KP_DIVIDE;
    case XK_KP_Multiply:    return GLFW_KEY_KP_MULTIPLY;
    case XK_KP_Subtract:    return GLFW_KEY_KP_SUBTRACT;
    case XK_KP_Add:         return GLFW_KEY_KP_ADD;
      
        // These should have been detected in secondary keysym test above!
    case XK_KP_Insert:      return GLFW_KEY_KP_0;
    case XK_KP_End:         return GLFW_KEY_KP_1;
    case XK_KP_Down:        return GLFW_KEY_KP_2;
    case XK_KP_Page_Down:   return GLFW_KEY_KP_3;
    case XK_KP_Left:        return GLFW_KEY_KP_4;
    case XK_KP_Right:       return GLFW_KEY_KP_6;
    case XK_KP_Home:        return GLFW_KEY_KP_7;
    case XK_KP_Up:          return GLFW_KEY_KP_8;
    case XK_KP_Page_Up:     return GLFW_KEY_KP_9;
    case XK_KP_Delete:      return GLFW_KEY_KP_DECIMAL;
    case XK_KP_Equal:       return GLFW_KEY_KP_EQUAL;
    case XK_KP_Enter:       return GLFW_KEY_KP_ENTER;

      // Last resort: Check for printable keys (should not happen if the XKB
      // extension is available). This will give a layout dependent mapping
      // (which is wrong, and we may miss some keys, especially on non-US
      // keyboards), but it's better than nothing...
    case XK_a:              return GLFW_KEY_A;
    case XK_b:              return GLFW_KEY_B;
    case XK_c:              return GLFW_KEY_C;
    case XK_d:              return GLFW_KEY_D;
    case XK_e:              return GLFW_KEY_E;
    case XK_f:              return GLFW_KEY_F;
    case XK_g:              return GLFW_KEY_G;
        case XK_h:              return GLFW_KEY_H;
        case XK_i:              return GLFW_KEY_I;
        case XK_j:              return GLFW_KEY_J;
        case XK_k:              return GLFW_KEY_K;
        case XK_l:              return GLFW_KEY_L;
        case XK_m:              return GLFW_KEY_M;
        case XK_n:              return GLFW_KEY_N;
        case XK_o:              return GLFW_KEY_O;
        case XK_p:              return GLFW_KEY_P;
        case XK_q:              return GLFW_KEY_Q;
        case XK_r:              return GLFW_KEY_R;
        case XK_s:              return GLFW_KEY_S;
        case XK_t:              return GLFW_KEY_T;
        case XK_u:              return GLFW_KEY_U;
        case XK_v:              return GLFW_KEY_V;
        case XK_w:              return GLFW_KEY_W;
        case XK_x:              return GLFW_KEY_X;
        case XK_y:              return GLFW_KEY_Y;
        case XK_z:              return GLFW_KEY_Z;
        case XK_1:              return GLFW_KEY_1;
        case XK_2:              return GLFW_KEY_2;
        case XK_3:              return GLFW_KEY_3;
        case XK_4:              return GLFW_KEY_4;
        case XK_5:              return GLFW_KEY_5;
        case XK_6:              return GLFW_KEY_6;
        case XK_7:              return GLFW_KEY_7;
        case XK_8:              return GLFW_KEY_8;
        case XK_9:              return GLFW_KEY_9;
        case XK_0:              return GLFW_KEY_0;
        case XK_space:          return GLFW_KEY_SPACE;
        case XK_minus:          return GLFW_KEY_MINUS;
        case XK_equal:          return GLFW_KEY_EQUAL;
        case XK_bracketleft:    return GLFW_KEY_LEFT_BRACKET;
        case XK_bracketright:   return GLFW_KEY_RIGHT_BRACKET;
        case XK_backslash:      return GLFW_KEY_BACKSLASH;
        case XK_semicolon:      return GLFW_KEY_SEMICOLON;
        case XK_apostrophe:     return GLFW_KEY_APOSTROPHE;
        case XK_grave:          return GLFW_KEY_GRAVE_ACCENT;
        case XK_comma:          return GLFW_KEY_COMMA;
        case XK_period:         return GLFW_KEY_PERIOD;
        case XK_slash:          return GLFW_KEY_SLASH;
        case XK_less:           return GLFW_KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }
  return GLFW_KEY_UNKNOWN;
}



int key_table[0xFFFF];

int glfw_to_x11_key(u32 key){
  if(key > 0xFFFF) return 0;
  if(key_table[KEY_UP] == 0){
    for(int i = 0; i < 0xFFFF; i++){
      int glfwkey = x11_to_glfw_key(i);
      if(glfwkey >= 0 && glfwkey < 0xFFFF)
	key_table[glfwkey] = i;
    }
  }
  return key_table[key];

}


void * get_handle_x11_win(Window win);
void x11_poll_events(){
  
  while(XPending(display)){
    XEvent event;
    XNextEvent(display, &event);
    
    
    int keytype;

    Window win;
    
    switch(event.type){
    case KeyPress:
      keytype = EVT_KEY_DOWN;
      win = event.xkey.window;
      break;
    case KeyRelease:
      keytype = EVT_KEY_UP;
      win = event.xkey.window;
      break;
    default:
      keytype = 0;
    }

    if(keytype){
      var h = get_handle_x11_win(win);

      int sym = 0;//XKeycodeToKeysym(display, event.xkey.keycode, 0);
      
      gl_window_event evt = {.key = {.key = x11_to_glfw_key(sym) , .ischar = false} };
      register_evt(h, &evt, keytype);

    }

  }
}


void x11_init(){
  XInitThreads();
  XrmInitialize();
  display = XOpenDisplay(NULL);
  root = DefaultRootWindow(display);
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  vi = glXChooseVisual(display, 0, att);
  cmap = XCreateColormap(display, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask;
}

void x11_deinit(){
  //pass
}


typedef struct{
  Window win;
  GLXContext glc;
}window_handle;


static int window_handle_cnt = 0;
window_handle ** window_handles;

void * get_handle_x11_win(Window win){
  for(int i = 0; i < window_handle_cnt; i++){
    if(window_handles[i]->win == win)
      return window_handles[i];
  }
  return NULL;

}

void * x11_create_window(int width, int height, const char * title){
  Window win = XCreateWindow(display, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

  XMapWindow(display, win);
  XStoreName(display, win, title);
  
  var glc = glXCreateContext(display, vi, NULL, GL_TRUE);
  window_handle * h = alloc0(sizeof(window_handle));
  h->win = win;
  h->glc = glc;
  window_handles = realloc(window_handles, (window_handle_cnt += 1) * sizeof(window_handles[0]));
  window_handles[window_handle_cnt - 1] = h;
  return h;
}

void x11_make_current(void * handle){
  window_handle * h = handle;
  glXMakeCurrent(display, h->win, h->glc);
}

void x11_swap_buffers(void * handle){
 window_handle * h = handle;
 glXSwapBuffers(display, h->win);
}

void x11_get_window_size(void * handle, int * w, int * _h){
  XWindowAttributes gwa;
  window_handle * h = handle;
  XGetWindowAttributes(display, h->win, &gwa);
  *w = gwa.width;
  *_h = gwa.height;
}

void x11_set_window_size(void * handle, int w, int h){
  ERROR("Not supported");
  //TODO: This might is wrong
  XWindowAttributes gwa;
  window_handle * _h = handle;
  XGetWindowAttributes(display, _h->win, &gwa);
  gwa.width = w;
  gwa.height = h;
  XChangeWindowAttributes(display, _h->win, 0, (XSetWindowAttributes *)&gwa);
}

void x11_get_cursor_position(void * handle, int * x, int * y){
  window_handle * h = handle;
  Window root, child;
  int root_x, root_y, win_x, win_y;
  u32 mask;
  XQueryPointer(display, h->win, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
  *x = win_x;
  *y = win_y;
}


bool x11_get_button_state(void * handle, int btn){
  UNUSED(handle);
  UNUSED(btn);
  return false;
}

bool x11_get_key_state(void * handle, int btn){
  int x11key = glfw_to_x11_key((u32)btn);

  UNUSED(handle);
  
  char keys[32];
  XQueryKeymap(display, keys);
  int btn2 = XKeysymToKeycode(display, x11key);
  int seq = btn2/8;

  if(seq < 0 ||  seq >= 32) return false;
  if(keys[seq]&(0x1<<(btn2%8)))
    return true;
  return false;
}

gl_backend * x11_create_backend(){
  gl_backend * backend = alloc0(sizeof(backend[0]));
  backend->poll_events = x11_poll_events;
  backend->init = x11_init;
  backend->deinit = x11_deinit;
  backend->create_window = x11_create_window;
  backend->make_current = x11_make_current;
  backend->swap_buffers = x11_swap_buffers;
  backend->get_window_size = x11_get_window_size;
  backend->get_cursor_position = x11_get_cursor_position;
  backend->get_button_state = x11_get_button_state;
  backend->get_key_state = x11_get_key_state;
  return backend;
}
