#include "full.h"
#include <GLFW/glfw3.h>
struct _gl_window{
  GLFWwindow * handle;
};

static bool glfwInited = false;

static void glfw_deinit(void * unused){
  UNUSED(unused);
  glfwTerminate();
  logd("GLFW terminated..\n");
}

static gl_window ** all_windows = NULL;
static int all_window_cnt = 0;

static gl_window_event * events = NULL;
static int event_cnt = 0;

size_t gl_get_events(gl_window_event * event_buffer, size_t max_read){
  glfwPollEvents();
  int to_read = MIN(max_read, (size_t)event_cnt);
  memcpy(event_buffer, events, to_read * sizeof(events[0]));
  memmove(events, events + to_read,  (event_cnt - to_read)* sizeof(events[0]));
  event_cnt = event_cnt - to_read;
  ASSERT(event_cnt >= 0);
  return to_read;
}

gl_window * window_from_handle(GLFWwindow * win){
  ASSERT(all_window_cnt > 0);
  for(int i = 0; i < all_window_cnt; i++)
    if(all_windows[i]->handle  == win)
      return all_windows[i];
  return NULL;
}

static void register_evt(GLFWwindow * win, void * _evt, gl_event_known_event_types type){
  gl_window_event * evt = _evt;
  evt->win = window_from_handle(win);
  evt->timestamp = timestamp();
  evt->type = type;
  list_push2(events, event_cnt, *evt);
}

void keycallback(GLFWwindow * win, int key, int scancode, int action, int mods){
  UNUSED(mods); UNUSED(scancode);
  
  evt_key keyevt = {.key = key , .ischar = false};
  register_evt(win, &keyevt, action ? EVT_KEY_DOWN : EVT_KEY_UP);
}

void charcallback(GLFWwindow * win, u32 codept){
  evt_key keyevt = {.codept = codept, .ischar = true};
  register_evt(win, &keyevt, EVT_KEY_DOWN);
}

void cursorposcallback(GLFWwindow * win, double x, double y){
  evt_mouse_move evt = {.x = x, .y = y};
  register_evt(win, &evt, EVT_MOUSE_MOVE);
}

void scrollcallback(GLFWwindow * win, double x, double y){
  evt_mouse_scroll evt= {.scroll_x = x, .scroll_y = y};
  register_evt(win, &evt, EVT_MOUSE_SCROLL);
}

void cursorentercallback(GLFWwindow * win, int enter){
  gl_window_event evt;
  register_evt(win, &evt, enter ? EVT_MOUSE_ENTER : EVT_MOUSE_LEAVE);
}

void mousebuttoncallback(GLFWwindow * win, int button, int action, int mods){
  UNUSED(mods);
  evt_mouse_btn btn = {.button = button};
  register_evt(win, &btn, action ? EVT_MOUSE_BTN_DOWN : EVT_MOUSE_BTN_UP);
}

gl_window * gl_window_open(i32 width, i32 height){
  if(!glfwInited){
    glfwInited = true;
    glfwInit();
    iron_register_deinitializer(glfw_deinit, NULL);
  }
  gl_window * win = alloc0(sizeof(gl_window));
  win->handle = glfwCreateWindow(width, height, "", NULL, NULL);
  glfwSetKeyCallback(win->handle, keycallback);
  glfwSetCharCallback(win->handle, charcallback);
  glfwSetMouseButtonCallback(win->handle, mousebuttoncallback);
  glfwSetCursorPosCallback(win->handle, cursorposcallback);
  glfwSetCursorEnterCallback(win->handle, cursorentercallback);
  glfwSetScrollCallback(win->handle, scrollcallback);

  list_push2(all_windows, all_window_cnt, win);
  
  return win;
}

void gl_window_make_current(gl_window * win){
  logd("handle: %p\n", win->handle);
  glfwMakeContextCurrent(win->handle);
}

void gl_window_swap(gl_window * win){
  glfwSwapBuffers(win->handle);
}

void gl_window_destroy(gl_window ** win){
  gl_window * _win = *win;
  *win = NULL;  
  glfwDestroyWindow(_win->handle);
  dealloc(_win);
}

void get_mouse_position(gl_window * win, int * x, int * y){
  double _x, _y;
  glfwGetCursorPos(win->handle, &_x, &_y);
  *x = _x;
  *y = _y;
}

static void ** deinitializers = NULL;
static void ** deinitializer_data = NULL;
static int deinitializer_cnt = 0;

void iron_register_deinitializer(void (* f)(void * data), void * data){
  list_push(deinitializer_data, deinitializer_cnt, data);
  list_push2(deinitializers, deinitializer_cnt, f);
}

void iron_deinitialize(){
  for(int i = 0; i < deinitializer_cnt; i++){
    void (* f)(void * data) = deinitializers[i];
    f(deinitializer_data[i]);
  }
  dealloc(deinitializers);
  dealloc(deinitializer_data);
  deinitializers = NULL;
  deinitializer_data = NULL;
  deinitializer_cnt = 0;
}

