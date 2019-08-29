#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GLFW/glfw3.h>

void keycallback(GLFWwindow * win, int key, int scancode, int action, int mods){
  UNUSED(mods); UNUSED(scancode);
  if(action == GLFW_REPEAT)
    return;
  evt_key keyevt = {.key = key , .ischar = false};
  register_evt(win, &keyevt, action == GLFW_PRESS ? EVT_KEY_DOWN : EVT_KEY_UP);
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

void windowclosecallback(GLFWwindow * win){
  gl_window_event evt;
  register_evt(win, &evt, EVT_WINDOW_CLOSE);
}

void glfw_poll_events(){
  glfwPollEvents();
}

void glfw_init(){
  glfwInit();
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
}

void glfw_deinit(){
  glfwTerminate();
}


void * glfw_create_window(int width, int height, const char * title){
  
  glfwWindowHint(GLFW_DEPTH_BITS, 16);
  GLFWwindow * handle = glfwCreateWindow(width, height, title, NULL, NULL);
  glfwSetKeyCallback(handle, keycallback);
  glfwSetCharCallback(handle, charcallback);
  glfwSetMouseButtonCallback(handle, mousebuttoncallback);
  glfwSetCursorPosCallback(handle, cursorposcallback);
  glfwSetCursorEnterCallback(handle, cursorentercallback);
  glfwSetScrollCallback(handle, scrollcallback);
  glfwSetWindowCloseCallback(handle, windowclosecallback);

  glfwMakeContextCurrent(handle);
  return handle;
}

void glfw_make_current(void * window){
  glfwMakeContextCurrent((GLFWwindow *) window);
}

void glfw_swap_buffers(void * window){
  glfwSwapBuffers(window);
}

void glfw_get_window_size (void * window, int * w, int * h){
  glfwGetWindowSize((GLFWwindow *) window, w, h);
}

void glfw_set_window_size(void * window, int w, int h){
  glfwSetWindowSize((GLFWwindow *) window, w, h);
}

void glfw_get_cursor_position(void * window, int * x, int * y){
  double _x, _y;
  glfwGetCursorPos((GLFWwindow *) window, &_x, &_y);
  *x = _x;
  *y = _y;
}

bool glfw_get_button_state(void * window, int btn){
  return glfwGetMouseButton((GLFWwindow *) window, btn) == GLFW_PRESS;
}

bool glfw_get_key_state(void * window, int key){
  return glfwGetKey((GLFWwindow *) window, key) == GLFW_PRESS;
}



gl_backend * glfw_create_backend(){
  gl_backend * backend = alloc0(sizeof(gl_backend));
  backend->poll_events = glfw_poll_events;
  backend->init = glfw_init;
  backend->deinit = glfw_deinit;
  backend->create_window = glfw_create_window;
  backend->make_current = glfw_make_current;
  backend->swap_buffers = glfw_swap_buffers;
  backend->get_window_size = glfw_get_window_size;
  backend->set_window_size = glfw_set_window_size;
  backend->get_cursor_position = glfw_get_cursor_position;
  backend->get_button_state = glfw_get_button_state;
  backend->get_key_state = glfw_get_key_state;
  return backend;
}
