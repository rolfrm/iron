#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GLFW/glfw3.h>

static void keycallback(GLFWwindow * win, int key, int scancode, int action, int mods){
  UNUSED(mods); UNUSED(scancode);
  gl_window_event evt = {.key = {.key = key ,.scancode = scancode, .ischar = false}};
  evt.key.key = key;
  //logd("KEY: %i %i\n", key, scancode);

  int keytype;
  switch(action){
  case GLFW_PRESS:
    keytype = EVT_KEY_DOWN;
    break;
  case GLFW_RELEASE:
    keytype = EVT_KEY_UP;
    break;
  case GLFW_REPEAT:
    keytype = EVT_KEY_REPEAT;
    break;
  default:
    return;

  }
  
  register_evt(win, &evt, keytype);
}

static void charcallback(GLFWwindow * win, u32 codept){
  gl_window_event evt = {.key = {.codept = codept, .ischar = true}};
  register_evt(win, &evt, EVT_KEY_DOWN);
}

static void cursorposcallback(GLFWwindow * win, double x, double y){
  gl_window_event evt = {.mouse_move = {.x = x, .y = y}};
  register_evt(win, &evt, EVT_MOUSE_MOVE);
}

static void scrollcallback(GLFWwindow * win, double x, double y){
  gl_window_event evt = {.mouse_scroll = {.x = x, .y = y}};
  register_evt(win, &evt, EVT_MOUSE_SCROLL);
}

static void cursorentercallback(GLFWwindow * win, int enter){
  gl_window_event evt;
  register_evt(win, &evt, enter ? EVT_MOUSE_ENTER : EVT_MOUSE_LEAVE);
}

static void mousebuttoncallback(GLFWwindow * win, int button, int action, int mods){
  UNUSED(mods);
  gl_window_event btn = {.mouse_btn = {.button = button}};
  register_evt(win, &btn, action ? EVT_MOUSE_BTN_DOWN : EVT_MOUSE_BTN_UP);
}

static void windowclosecallback(GLFWwindow * win){
  gl_window_event evt;
  register_evt(win, &evt, EVT_WINDOW_CLOSE);
}

static void windowsizecallback(GLFWwindow * win, int width, int height){
  gl_window_event evt = {.window_size_change = {.width = width, .height = height}};
  register_evt(win, &evt, EVT_WINDOW_RESIZE);
}

static void window_pos_callback(GLFWwindow* win, int xpos, int ypos)
{
  gl_window_event evt = {.window_position_change = {.x = xpos, .y = ypos}};
  register_evt(win, &evt, EVT_WINDOW_MOVE);
}


static void glfw_poll_events(void){
  glfwPollEvents();
}

static void glfw_init(void){
  glfwInit();
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  //glfwSwapInterval(1);
}

static void glfw_deinit(void){
  glfwTerminate();
}


static void glfw_debug_print (GLenum _source, 
                            GLenum _type, 
                            GLuint id, 
                            GLenum _severity, 
                            GLsizei length, 
                            const GLchar *message, 
                            void *userParam)
{
  UNUSED(length);
  UNUSED(userParam);
  
    // ignore non-significant error/warning codes
    //if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 
    const char * source = "_";
    
    switch (_source)
      {
      case GL_DEBUG_SOURCE_API:             source = "API"; break;
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source = "Window System"; break;
      case GL_DEBUG_SOURCE_SHADER_COMPILER: source = "Shader Compiler"; break;
      case GL_DEBUG_SOURCE_THIRD_PARTY: source = "Third Party"; break;
      case GL_DEBUG_SOURCE_APPLICATION: source = "Application"; break;
      case GL_DEBUG_SOURCE_OTHER: source =  "Other"; break;
      }
    const char * type = "_";
    switch (_type)
    {
        case GL_DEBUG_TYPE_ERROR:               type ="Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type ="Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type ="Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         type ="Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         type ="Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              type ="Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          type ="Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           type ="Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               type ="Other"; break;
    };
    const char * severity = "_";
    switch (_severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         severity = "high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severity = "medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          severity = "low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severity = "notification"; break;
    }

    logd("GL DEBUG %s %s %s\n", severity, type, source);
    logd("%i: %s\n", id, message);
}


static void errorcallback(int errid, const char * err){
  
  printf("GLFW %i: %s\n", errid, err);

}

void * glfw_create_window(int width, int height, const char * title){
  static GLFWwindow * main_context = NULL;
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  
  GLFWwindow * handle = glfwCreateWindow(width, height, title, NULL, main_context);
  if(main_context == NULL)
    main_context = handle;
  glfwSetKeyCallback(handle, keycallback);
  glfwSetCharCallback(handle, charcallback);
  glfwSetMouseButtonCallback(handle, mousebuttoncallback);
  glfwSetCursorPosCallback(handle, cursorposcallback);
  glfwSetCursorEnterCallback(handle, cursorentercallback);
  glfwSetScrollCallback(handle, scrollcallback);
  glfwSetWindowCloseCallback(handle, windowclosecallback);
  glfwSetErrorCallback(errorcallback);
  glfwSetWindowSizeCallback(handle, windowsizecallback);
  glfwSetWindowPosCallback(handle, window_pos_callback);
  

  glfwMakeContextCurrent(handle);

  if(iron_gl_debug){
    logd("GL DEBUG: Enable OPENGL Debug context\n");
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#ifndef __EMSCRIPTEN__

    glDebugMessageCallback((void *)glfw_debug_print, NULL);
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
  }
  
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

void glfw_destroy_window(void * window){
  glfwDestroyWindow((GLFWwindow *)window);
}

bool glfw_get_button_state(void * window, int btn){
  return glfwGetMouseButton((GLFWwindow *) window, btn) == GLFW_PRESS;
}

bool glfw_get_key_state(void * window, int key){
  int st = glfwGetKey((GLFWwindow *) window, key) ;
  //logd("Key down? %i %i %i\n", key, st, GLFW_KEY_A);
  return st;
  
}

static GLFWcursor * normal_cursor = NULL;

void glfw_set_cursor_type(void * window, iron_cursor_type type){
  static GLFWcursor * custom_cursor;
  var win = (GLFWwindow *) window;
  int glfwCursor;
  GLFWcursor* cursor=  NULL;
  switch(type){
  case IRON_CURSOR_NORMAL:
    glfwCursor = GLFW_ARROW_CURSOR;
    break;
  case IRON_CURSOR_CROSSHAIR:
    glfwCursor = GLFW_CROSSHAIR_CURSOR;
    if(custom_cursor == NULL){
      unsigned char pixels[8 * 8 * 4] = {0};
      u32 * px = (u32 *) pixels;

      int wpos[] = {1,1, 2,1,3,1, 1,2,1,3};
      int bpos[] = {2,2, 3,2,4,2, 2,3,2,4};
      
      for(u32 i = 0; i < array_count(wpos) / 2; i++){
	int index = wpos[i * 2] * 8 + wpos[i * 2 + 1];
	px[index] = 0xFFFFFFFF;
      }
      for(u32 i = 0; i < array_count(bpos) / 2; i++){
	int index = bpos[i * 2] * 8 + bpos[i * 2 + 1];
	px[index] = 0xFF000000;
      }
      
      
      GLFWimage image;
      image.width = 8;
      image.height = 8;
      image.pixels = pixels;
      custom_cursor = glfwCreateCursor(&image, 0, 0);
    }
    //glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    cursor = custom_cursor;
    printf("CROSSHAIR CURSOR %p\n", cursor);
    break;
  default:
    ERROR("INVALID CURSOR");
    return;
  }
  if(cursor != NULL){
    printf("Set custom  CURSOR %p\n", cursor);
    glfwSetCursor(win, cursor);
  }else{
    normal_cursor = glfwCreateStandardCursor(glfwCursor);
    glfwSetCursor(win, normal_cursor);
  }
  
}

static void glfw_show_cursor(void * window, bool show){
  if(show){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }else{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }
}

static void glfw_set_window_position(void * window, int x, int y){
  glfwSetWindowPos((GLFWwindow *)window, x, y);
}

static void glfw_get_window_position(void * window, int * x, int * y){
  glfwGetWindowPos((GLFWwindow *)window, x, y);
}

static const char * glfw_get_clipboard(void * window){
  return glfwGetClipboardString((GLFWwindow *)window);
}

static void glfw_set_window_title(void * window, const char * new_title){
  glfwSetWindowTitle((GLFWwindow *) window, new_title);
}

gl_backend * glfw_create_backend(void){
  gl_backend * backend = alloc0(sizeof(gl_backend));
  backend->poll_events = glfw_poll_events;
  backend->init = glfw_init;
  backend->deinit = glfw_deinit;
  backend->create_window = glfw_create_window;
  backend->destroy_window = glfw_destroy_window;
  backend->make_current = glfw_make_current;
  backend->swap_buffers = glfw_swap_buffers;
  backend->get_window_size = glfw_get_window_size;
  backend->set_window_size = glfw_set_window_size;
  backend->get_window_position = glfw_get_window_position;
  backend->set_window_position = glfw_set_window_position;
  backend->get_cursor_position = glfw_get_cursor_position;
  backend->get_button_state = glfw_get_button_state;
  backend->get_key_state = glfw_get_key_state;
  backend->set_cursor_type = glfw_set_cursor_type;
  backend->show_cursor = glfw_show_cursor;
  backend->get_clipboard = glfw_get_clipboard;
  backend->set_window_title = glfw_set_window_title;
  return backend;
}
