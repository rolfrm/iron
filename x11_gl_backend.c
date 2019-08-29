#include "full.h"
#include "gl.h"
#include <X11/Xresource.h>
#include<GL/glx.h>

static Display * display;
static Window root;
static XVisualInfo * vi;
static Colormap cmap;
static XSetWindowAttributes swa;
void x11_poll_events(){
  
  while(XPending(display)){
    XEvent event;
    XNextEvent(display, &event);
    switch(event.type){
    case KeyPress:
    case KeyRelease:
      printf("key press\n");
      break;
      

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

void * x11_create_window(int width, int height, const char * title){
  Window win = XCreateWindow(display, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

  XMapWindow(display, win);
  XStoreName(display, win, title);
  
  var glc = glXCreateContext(display, vi, NULL, GL_TRUE);
  window_handle * h = alloc0(sizeof(window_handle));
  h->win = win;
  h->glc = glc;
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
  XChangeWindowAttributes(display, _h->win, 0, &gwa);
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
  UNUSED(handle);
  char keys[32];
  XQueryKeymap(display, keys);
  int btn2 = XKeysymToKeycode(display, btn);
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
