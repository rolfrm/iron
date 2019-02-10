#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GLFW/glfw3.h>

#include <GL/glext.h>
#include "stb_image.h"
#include "texture.shader.c"

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

gl_window * gl_window_open(i32 width, i32 height){
  if(!glfwInited){
    glfwInited = true;
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
    iron_register_deinitializer(glfw_deinit, NULL);
  }
  gl_window * win = alloc0(sizeof(gl_window));
  win->handle = glfwCreateWindow(width, height, "", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  glfwSetKeyCallback(win->handle, keycallback);
  glfwSetCharCallback(win->handle, charcallback);
  glfwSetMouseButtonCallback(win->handle, mousebuttoncallback);
  glfwSetCursorPosCallback(win->handle, cursorposcallback);
  glfwSetCursorEnterCallback(win->handle, cursorentercallback);
  glfwSetScrollCallback(win->handle, scrollcallback);
  glfwSetWindowCloseCallback(win->handle, windowclosecallback);

  list_push2(all_windows, all_window_cnt, win);
  
  return win;
}

gl_window * current_window;

void gl_window_make_current(gl_window * win){
  //logd("handle: %p\n", win->handle);
  glfwMakeContextCurrent(win->handle);
  current_window = win;
}

void gl_window_swap(gl_window * win){
  glfwSwapBuffers(win->handle);
}

void gl_window_destroy(gl_window ** win){
  gl_window * _win = *win;
  *win = NULL;  
  glfwDestroyWindow(_win->handle);
  for(int i = 0; i < all_window_cnt; i++){
    if(all_windows[i] == _win){
      list_remove2(all_windows, all_window_cnt, i);
      break;
    }
  }
  
  dealloc(_win);
}

void gl_window_get_size(gl_window * win, int *w, int *h){
  glfwGetWindowSize(win->handle, w, h);
}

void gl_window_poll_events(){
  glfwPollEvents();
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

typedef enum{
  IMAGE_SOURCE_MEMORY,
  IMAGE_SOURCE_FILE

}image_source_kind;

struct _image_source{
  image_source_kind kind;
  void * data;
};

image image_from_file(const char * path){
  int _x = 0, _y = 0, _c = 0;
  void * imgdata = stbi_load(path, &_x, &_y, &_c, 4);
  logd("LOADED: %i\n", _x);
  image_source src;
  src.kind = IMAGE_SOURCE_MEMORY;
  src.data = imgdata;

  image img = {.width = _x, .height = _y, .channels = _c, .source = IRON_CLONE(src)};
  return img;
}


image image_from_data(void * data, int len){
  int _x = 0, _y = 0, _c = 0;
  void * imgdata = stbi_load_from_memory(data, len, &_x, &_y, &_c, 4);
  image_source src;
  src.kind = IMAGE_SOURCE_MEMORY;
  src.data = imgdata;

  image img = {.width = _x, .height = _y, .channels = _c, .source = IRON_CLONE(src)};
  return img;
}


void * image_data(image * image){
  return image->source->data;
}

void image_delete(image * img){
  if(img->source->data != NULL)
    dealloc(img->source->data);
  dealloc(img->source);
  image im2 = {0};
  img[0] = im2;
}

struct _texture_handle {
  GLuint tex;
};

texture texture_from_image(image * image){
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  int chn[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
  void * data = image_data(image);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, chn[image->channels - 1], GL_UNSIGNED_BYTE, data);
  
  texture_handle hndl = {.tex = tex};
  texture texture = { .handle = IRON_CLONE(hndl), .width = image->width, .height = image->height};
  return texture;
}


u32 compile_shader(int shader_type, const char * code){
  u32 ss = glCreateShader(shader_type);
  i32 l = strlen(code);
  logd("CODE: %s\n", code);
  glShaderSource(ss, 1, (void *) &code, &l); 
  glCompileShader(ss);
  int compileStatus = 0;	
  glGetShaderiv(ss, GL_COMPILE_STATUS, &compileStatus);
  if(compileStatus == GL_FALSE){
    logd("Error during shader compilation:\n");
    int loglen = 0;
    glGetShaderiv(ss, GL_INFO_LOG_LENGTH, &loglen);
    char * buffer = alloc0(loglen + 10);
    glGetShaderInfoLog(ss, loglen, &loglen, buffer);
    buffer[loglen] = 0;
    logd("%i: '%s'\n", loglen, buffer);
    dealloc(buffer);
  } else{
    logd("Compiled shader with success\n");
  }
  return ss;
}

u32 compile_shader_from_file(u32 gl_prog_type, const char * filepath){
  char * vcode = read_file_to_string(filepath);
  u32 vs = compile_shader(gl_prog_type, vcode);
  dealloc(vcode);
  return vs;
}

u32 create_shader_from_codes(const char * vsc, const char * fsc){
  u32 vs = compile_shader(GL_VERTEX_SHADER, vsc);
  u32 fs = compile_shader(GL_FRAGMENT_SHADER, fsc);
  u32 prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  return prog;
}

u32 create_shader_from_files(const char * vs_path, const char * fs_path){
  char * vscode = read_file_to_string(vs_path);
  char * fscode = read_file_to_string(fs_path);
  u32 prog = create_shader_from_codes(vscode, fscode);
  dealloc(vscode);
  dealloc(fscode);
  return prog;
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  //if(type == GL_DEBUG_TYPE_ERROR)
    ERROR("GL ERROR: %s", message);
  
}

typedef struct _textured_shader{
  int vertex_transform_loc, uv_transform_loc, texture_loc, textured_loc, color_loc;
  int pos_attr, tex_coord_attr;
  int blit_shader;
  
}textured_shader;

static textured_shader shader;
static mat3 blit_transform;
void blit_begin(){
  if(shader.blit_shader == 0){
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION,
                              GL_DONT_CARE,
                              GL_DONT_CARE,
                              0,
                              NULL,
                              GL_TRUE);
    //glDisable( GL_DEBUG_MESSAGE_CONTROL, GL_FALSE);
    glDebugMessageCallback( MessageCallback, 0 );
    shader.blit_shader = create_shader_from_codes(texture_shader_vs, texture_shader_fs);

    shader.pos_attr = 0;
    shader.tex_coord_attr = 1;
    shader.vertex_transform_loc = glGetUniformLocation(shader.blit_shader, "vertex_transform");
    shader.uv_transform_loc = glGetUniformLocation(shader.blit_shader, "uv_transform");
    shader.texture_loc = glGetUniformLocation(shader.blit_shader, "texture");
    shader.color_loc = glGetUniformLocation(shader.blit_shader, "color");
    shader.textured_loc = glGetUniformLocation(shader.blit_shader, "textured");
    glUseProgram(shader.blit_shader);
  }
  glUseProgram(shader.blit_shader);

  
  mat3 identity = mat3_identity();
  blit_transform = identity;
  blit_translate(-1,-1);
  blit_scale(2,2);
  int w,h;

  gl_window_get_size(current_window, &w, &h);
  blit_scale(1.0f / (float)w, 1.0f/ (float)h);
  ;
  glUniformMatrix3fv(shader.uv_transform_loc, 1, false, &identity.m00);
  glUniform1i(shader.texture_loc, 0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

void blit(float x,float y, texture * tex){
  blit_translate(x,y);
  blit_scale(tex->width, tex->height);

  glUniformMatrix3fv(shader.vertex_transform_loc, 1, false, &blit_transform.m00);
  glBindTexture(GL_TEXTURE_2D, tex->handle->tex);
  //printf("Binding texture: %i\n", tex->handle->tex);
  glUniform1i(shader.textured_loc, 1);
  float points[] = {0,0, 1,0, 0,1,1,1};
  float uvs[] = {0,1, 1,1, 0,0, 1,0};
  glVertexAttribPointer(shader.pos_attr, 2, GL_FLOAT, GL_FALSE, 0, points);
  glVertexAttribPointer(shader.tex_coord_attr, 2, GL_FLOAT, GL_FALSE, 0, uvs);

  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  glBindTexture(GL_TEXTURE_2D, 0);

  blit_scale(1.0f / tex->width, 1.0f/ tex->height);
  blit_translate(-x,-y);
}

void blit_rectangle(float x, float y, float w, float h, float r, float g, float b, float a){
  blit_translate(x,y);
  blit_scale(w, h);
  glUniformMatrix3fv(shader.vertex_transform_loc, 1, false, &blit_transform.m00);
  float points[] = {0,0, 1,0, 0,1,1,1};
  glUniform1i(shader.textured_loc, 0);
  glUniform4f(shader.color_loc, r, g, b, a);
  float uvs[] = {0,0,0,0,0,0,0,0};
  glVertexAttribPointer(shader.pos_attr, 2, GL_FLOAT, GL_FALSE, 0, points);
  glVertexAttribPointer(shader.tex_coord_attr, 2, GL_FLOAT, GL_FALSE, 0, uvs);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);

  blit_scale(1.0f/w, 1.0f/h);
  blit_translate(-x,-y);
}

void blit_translate(float x, float y){
  blit_transform = mat3_mul(blit_transform, mat3_2d_translation(x, y));
  
}
void blit_scale(float x, float y){
  blit_transform = mat3_mul(blit_transform, mat3_2d_scale(x, y));
}
