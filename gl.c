#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include "stb_image.h"
#include "texture.shader.c"

struct _gl_window{
  void * handle;
};
gl_backend * glfw_create_backend();
gl_backend * x11_create_backend();
static bool backend_initialized = false;
gl_backend * current_backend = NULL;
IRON_GL_BACKEND iron_gl_backend = IRON_GL_BACKEND_GLFW;

static gl_window ** all_windows = NULL;
static int all_window_cnt = 0;

static gl_window_event * events = NULL;
static int event_cnt = 0;

size_t gl_get_events(gl_window_event * event_buffer, size_t max_read){
  current_backend->poll_events();
  int to_read = MIN(max_read, (size_t)event_cnt);
  memcpy(event_buffer, events, to_read * sizeof(events[0]));
  memmove(events, events + to_read,  (event_cnt - to_read)* sizeof(events[0]));
  event_cnt = event_cnt - to_read;
  ASSERT(event_cnt >= 0);
  return to_read;
}

gl_window * window_from_handle(void * win){
  ASSERT(all_window_cnt > 0);
  for(int i = 0; i < all_window_cnt; i++)
    if(all_windows[i]->handle  == win)
      return all_windows[i];
  return NULL;
}

void register_evt(void * win, void * _evt, gl_event_known_event_types type){
  gl_window_event * evt = _evt;
  evt->win = window_from_handle(win);
  evt->timestamp = timestamp();
  evt->type = type;
  list_push2(events, event_cnt, *evt);
}


gl_window * gl_window_open(i32 width, i32 height){
  if(!backend_initialized){
    if(iron_gl_backend == IRON_GL_BACKEND_GLFW)
      current_backend = glfw_create_backend();
    else if(iron_gl_backend == IRON_GL_BACKEND_X11)
      current_backend = x11_create_backend();
    current_backend->init();
    backend_initialized = true;
  }
  gl_window * win = alloc0(sizeof(gl_window));
  win->handle = current_backend->create_window(width, height, "");

  list_push2(all_windows, all_window_cnt, win);
  
  return win;
}

gl_window * current_window;

void gl_window_make_current(gl_window * win){
  //logd("handle: %p\n", win->handle);
  current_backend->make_current(win->handle);
  
  int win_width, win_height;
  gl_window_get_size(win, &win_width, &win_height);
  glViewport(0,0,win_width, win_height);
  current_window = win;
}

void gl_window_swap(gl_window * win){
  current_backend->swap_buffers(win->handle);
}

void gl_window_destroy(gl_window ** win){
  gl_window * _win = *win;
  *win = NULL;
  current_backend->destroy_window(_win->handle);
  for(int i = 0; i < all_window_cnt; i++){
    if(all_windows[i] == _win){
      list_remove2(all_windows, all_window_cnt, i);
      break;
    }
  }
  
  dealloc(_win);
}

void gl_window_get_size(gl_window * win, int *w, int *h){
  current_backend->get_window_size(win->handle, w, h);
}

void gl_window_poll_events(){
  current_backend->poll_events();
}

void get_mouse_position(gl_window * win, int * x, int * y){
  current_backend->get_cursor_position(win->handle, x, y);
}

bool gl_window_get_btn_state(gl_window * win, int btn){
  if(current_backend->get_button_state == NULL)
    return false;
  return current_backend->get_button_state(win->handle, btn);
}

bool gl_window_get_key_state(gl_window * win, int key){
  if(current_backend->get_key_state == NULL)
    return false;
  return current_backend->get_key_state(win->handle, key);
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

image image_new(int width, int height, int channels){

  image_source src;
  src.kind = IMAGE_SOURCE_MEMORY;
  src.data = alloc0(width * height * channels);

  image img = {.width = width, .height = height, .channels = channels, .source = IRON_CLONE(src)};
  return img;
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

texture texture_from_image2(image * image, TEXTURE_INTERPOLATION interp){
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  int interp2;
  if(interp == TEXTURE_INTERPOLATION_BILINEAR){
    interp2 = GL_LINEAR_MIPMAP_LINEAR;
  }else{
    interp2 = GL_NEAREST;
  }


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp2);


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  int chn[] = {GL_R, GL_RG, GL_RGB, GL_RGBA};
  void * data = image_data(image);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, chn[image->channels - 1], GL_UNSIGNED_BYTE, data);
  if(interp == TEXTURE_INTERPOLATION_BILINEAR)
    glGenerateMipmap(GL_TEXTURE_2D);
  texture_handle hndl = {.tex = tex};
  texture texture = { .handle = IRON_CLONE(hndl), .width = image->width, .height = image->height};
  return texture;
}
texture texture_from_image(image * image){
  return texture_from_image2(image, TEXTURE_INTERPOLATION_BILINEAR);
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
/*
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
  
    }*/

typedef struct _textured_shader{
  int vertex_transform_loc, uv_transform_loc, texture_loc, textured_loc, color_loc;
  int pos_attr, tex_coord_attr;
  int blit_shader;
  
}textured_shader;

static textured_shader shader;
static mat3 blit_transform;
static BLIT_MODE blit_mode;

mat3 blit_get_view_transform(){
  return blit_transform;
}

void blit_begin(BLIT_MODE _blit_mode){
  blit_mode = _blit_mode;
  if(shader.blit_shader == 0){
    glEnable( GL_DEBUG_OUTPUT );
    /*glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION,
                              GL_DONT_CARE,
                              GL_DONT_CARE,
                              0,
                              NULL,
                              GL_TRUE);
    //glDisable( GL_DEBUG_MESSAGE_CONTROL, GL_FALSE);
    glDebugMessageCallback( MessageCallback, 0 );*/
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
  
  //blit_translate(1,1);
  //blit_scale(0.5,0.5);
  int w,h;
  if(blit_mode == BLIT_MODE_PIXEL){
    gl_window_get_size(current_window, &w, &h);
    blit_scale(1.0f / (float)w, 1.0f/ (float)h);
  }
  
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
  if(blit_mode == BLIT_MODE_PIXEL){
    blit_scale(tex->width, tex->height);
  }

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
  if(blit_mode == BLIT_MODE_PIXEL){
    blit_scale(1.0f / tex->width, 1.0f/ tex->height);
  }
  blit_translate(-x,-y);
}

void blit_rectangle(float x, float y, float w, float h, float r, float g, float b, float a){
  var t = blit_transform;
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

  blit_transform = t;
}

void blit_translate(float x, float y){
  blit_transform = mat3_mul(blit_transform, mat3_2d_translation(x, y) );
  
}
void blit_scale(float x, float y){
  blit_transform = mat3_mul(blit_transform, mat3_2d_scale(x, y));
}

