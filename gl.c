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

gl_window * gl_window_open(i32 width, i32 height){
  if(!glfwInited){
    glfwInited = true;
    glfwInit();
    iron_register_deinitializer(glfw_deinit, NULL);
  }
  gl_window * win = alloc0(sizeof(gl_window));
  win->handle = glfwCreateWindow(width, height, "", NULL, NULL);
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
