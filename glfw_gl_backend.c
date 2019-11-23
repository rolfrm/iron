#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GLFW/glfw3.h>

void keycallback(GLFWwindow * win, int key, int scancode, int action, int mods){
  UNUSED(mods); UNUSED(scancode);
  evt_key keyevt = {.key = key , .ischar = false};

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
  
  register_evt(win, &keyevt, keytype);
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

static void glfw_debug_print (GLenum _source, 
                            GLenum _type, 
                            GLuint id, 
                            GLenum _severity, 
                            GLsizei length, 
                            const GLchar *message, 
                            void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 
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

    logd("GL DEBUG %s %s %s", severity, type, source);
    logd("%i: %s", id, message);
}


void * glfw_create_window(int width, int height, const char * title){
  
  glfwWindowHint(GLFW_DEPTH_BITS, 16);
  if(iron_gl_debug){
    logd("GL DEBUG: Enable OPENGL Debug context\n");
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    //glDebugMessageCallback(glfw_debug_print, NULL);
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
  }
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