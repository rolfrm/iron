#include "full.h"
#include <GLFW/glfw3.h>
struct _gl_window{
  GLFWwindow * handle;
};

static bool glfwInited = false;

gl_window * gl_window_open(i32 width, i32 height){
  if(!glfwInited){
    glfwInited = true;
    glfwInit();
  }
  gl_window * win = alloc0(sizeof(gl_window));
  win->handle = glfwCreateWindow(width, height, "", NULL, NULL);
  return win;
}

void gl_window_make_current(gl_window * win){
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

void gl_window_terminate(){
  glfwTerminate();
}
