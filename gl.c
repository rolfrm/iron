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
bool iron_gl_debug = false;
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
    if(all_windows[i]->handle == win)
      return all_windows[i];
  return NULL;
}

void register_evt(void * win, gl_window_event * evt, gl_event_known_event_types type){
  evt->win = window_from_handle(win);
  evt->timestamp = timestamp();
  evt->type = type;
  list_push2(events, event_cnt, *evt);
}


gl_window * gl_window_open(i32 width, i32 height){
  if(!backend_initialized){
#ifndef __EMSCRIPTEN__
    stbi_set_flip_vertically_on_load(1);
#endif
    if(iron_gl_backend == IRON_GL_BACKEND_GLFW)
      current_backend = glfw_create_backend();
#ifndef __EMSCRIPTEN__
    else if(iron_gl_backend == IRON_GL_BACKEND_X11)
      current_backend = x11_create_backend();
#endif
    current_backend->init();
    backend_initialized = true;
  }
  gl_window * win = alloc0(sizeof(gl_window));
  win->handle = current_backend->create_window(width, height, "OpenGL Window");

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

void gl_window_set_size(gl_window * win, int w, int h){
  ASSERT(current_backend->set_window_size != NULL);
  current_backend->set_window_size(win->handle, w, h);
}

void gl_window_set_position(gl_window * win, int x, int y){
  if(current_backend->set_window_position != NULL)
    current_backend->set_window_position(win->handle, x, y);
}
void gl_window_get_position(gl_window * win, int *x, int * y){
  if(current_backend->get_window_position != NULL)
    current_backend->get_window_position(win->handle, x, y);
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


const char * gl_window_get_clipboard(gl_window * win){
  if(current_backend->get_clipboard == NULL) return NULL;
  return current_backend->get_clipboard(win->handle);
}
void gl_window_set_title(gl_window * win, const char * title){
  if(current_backend->set_window_title == NULL) return;
  current_backend->set_window_title(win->handle, title);
}

void gl_window_set_cursor_type(gl_window * win, iron_cursor_type type){
  var f = current_backend->set_cursor_type;
  if(f == NULL) return;
  f(win->handle, type);
}

void gl_window_show_cursor(gl_window * win, bool show){
  var f = current_backend->show_cursor;
 if(f == NULL) return;
  f(win->handle, show);
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

image image_from_bitmap(void * bitmap, int width, int height, int channels){
  image_source src;
  src.kind = IMAGE_SOURCE_MEMORY;
  src.data = iron_clone(bitmap, width * height * channels);
  
  image img = {.width = width, .height = height, .channels = channels, .source = IRON_CLONE(src)};
  return img;  
}

image image_from_file(const char * path){
  int _x = 0, _y = 0, _c = 0;
  // note, if there are 3 channels or less, only power-of-2 textures are supported.
  // So images needs to be prepared for rendering otherwise.
  // this is why, for now, I just force it to be 4 channels always.
  // for performance another choice should be selected.
  void * imgdata = stbi_load(path, &_x, &_y, &_c, 4);
  return image_from_bitmap(imgdata, _x, _y, 4);
}

image image_from_data(void * data, int len){
  int _x = 0, _y = 0, _c = 0;
  void * imgdata = stbi_load_from_memory(data, len, &_x, &_y, &_c, 4);
  return image_from_bitmap(imgdata, _x, _y, _c);
}

void * image_data(image * image){
  if(image->source == NULL)
    return NULL;
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

static u32 gl_tex_interp(TEXTURE_INTERPOLATION interp, bool ismag){
  if(interp == TEXTURE_INTERPOLATION_BILINEAR){
    if(ismag)
      return GL_LINEAR;
    return GL_LINEAR_MIPMAP_LINEAR;
  }else if(interp == TEXTURE_INTERPOLATION_LINEAR){
    return GL_LINEAR;
  }else{
    return GL_NEAREST;
  }

}
texture texture_new(TEXTURE_INTERPOLATION sub_interp, TEXTURE_INTERPOLATION super_interp){

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);  

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_tex_interp(sub_interp, false));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_tex_interp(super_interp, true));


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  texture_handle hndl = {.tex = tex};
  texture texture = { .handle = IRON_CLONE(hndl), .width = 0, .height = 0};
  return texture;
}

void texture_load_image(texture * texture, image * image){
  gl_texture_bind(*texture);
  int chn[] = {GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
  int int_format[] ={GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
  u8 * data = image_data(image);
  bool delete_data = false;
  if(image->mode == GRAY_AS_ALPHA && image->channels == 1){
    chn[0] = GL_LUMINANCE_ALPHA;
    int_format[0] = GL_LUMINANCE_ALPHA;
    // cannot do swizzle as it is not supported in webgl.
    // instead provide a temporary LUMIANCE_ALPHA texture.
    if(data != NULL){
      int pixels = image->width * image->height;
      u8 * newdata = alloc0(2 * pixels);
      for(u32 i = 0; i < pixels; i++){
	newdata[i * 2] = 0xFF;
	newdata[i * 2 + 1] = data[i];
      }
      data = newdata;
      delete_data = true;
    }
  }
  
  if(data == NULL){
    glTexImage2D(GL_TEXTURE_2D, 0, int_format[image->channels - 1],
		 image->width, image->height, 0, chn[image->channels - 1], GL_UNSIGNED_BYTE, NULL);
  }else{
    glTexImage2D(GL_TEXTURE_2D, 0, int_format[image->channels - 1],
		 image->width, image->height, 0, chn[image->channels - 1], GL_UNSIGNED_BYTE, data);
  }
  texture->width = image->width;
  texture->height = image->height;
  if(delete_data)
    dealloc(data);
}

texture texture_from_image3(image * image, TEXTURE_INTERPOLATION sub_interp, TEXTURE_INTERPOLATION super_interp){
  var texture = texture_new(sub_interp, super_interp);
  texture_load_image(&texture, image);
  if(sub_interp == TEXTURE_INTERPOLATION_BILINEAR || super_interp == TEXTURE_INTERPOLATION_BILINEAR)
    glGenerateMipmap(GL_TEXTURE_2D);
  return texture;
}

texture texture_from_image2(image * image, TEXTURE_INTERPOLATION interp){
  return texture_from_image3(image, interp, interp);
}

texture texture_from_image(image * image){
  return texture_from_image2(image, TEXTURE_INTERPOLATION_BILINEAR);
}

void gl_texture_bind(texture tex){
  glBindTexture(GL_TEXTURE_2D, tex.handle->tex);
}


u32 compile_shader(int shader_type, const char * code){
  u32 ss = glCreateShader(shader_type);
  i32 l = strlen(code);
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
    printf("%i: '%s'\n", loglen, buffer);
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

u32 gl_shader_compile(const char * vsc, const char * fsc){
  u32 vs = compile_shader(GL_VERTEX_SHADER, vsc);
  u32 fs = compile_shader(GL_FRAGMENT_SHADER, fsc);
  u32 prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  return prog;
}

u32 gl_shader_compile2(const char * vsc, int vlen, const char * fsc, int flen){
  void * buffer1 = alloc0(vlen + 1);
  memcpy(buffer1, vsc, vlen);
  void * buffer2 = alloc0(flen + 1);
  memcpy(buffer2, fsc, flen);  
  u32 result = gl_shader_compile(buffer1, buffer2);
  dealloc(buffer1);
  dealloc(buffer2);
  return result;
}

u32 create_shader_from_files(const char * vs_path, const char * fs_path){
  char * vscode = read_file_to_string(vs_path);
  char * fscode = read_file_to_string(fs_path);
  u32 prog = gl_shader_compile(vscode, fscode);
  dealloc(vscode);
  dealloc(fscode);
  return prog;
}

static void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  if(type == GL_DEBUG_TYPE_ERROR)
    ERROR("GL ERROR: %i %i %s\n", severity, type, message);
  
}

typedef struct _textured_shader{
  int vertex_transform_loc, uv_transform_loc, texture_loc, textured_loc, color_loc;
  int pos_attr, tex_coord_attr;
  int blit_shader;
  
}textured_shader;

static u32 quadbuffer;
static u32 quadbuffer_uvs;
static textured_shader shader;

static mat3 blit_transform;
static mat3 blit_uv_transform;
static BLIT_MODE blit_mode;
static texture * blit_current_texture = NULL;
static vec4 _blit_color;

struct{
  mat3 blit_transform;
  mat3 uv_transform;
  BLIT_MODE blit_mode;

}blit_stack[10];

int current = -1;

BLIT_MODE blit_mode_get(){
  return blit_mode;
}

void blit_push(){
  if(current == 9) ERROR("Unable to push blit transforms: To many on stack.");
  current += 1;
  blit_stack[current].blit_transform = blit_transform;
  blit_stack[current].blit_mode = blit_mode;
  blit_stack[current].uv_transform = blit_uv_transform;
}


void blit_pop(){
  if(current < 0) ERROR("Unable to pop blit transforms: Stack empty.");
  //blit_begin(blit_mode);
  blit_transform = blit_stack[current].blit_transform;
  blit_mode = blit_stack[current].blit_mode;
  blit_uv_transform = blit_stack[current].uv_transform;  
  current -= 1;
}

mat3 blit_get_view_transform(){
  return blit_transform;
}

vec2 blit_translate_point(vec2 p){
  return mat3_mul_vec3(blit_transform, vec3_new(p.x, p.y, 1)).xy;

}
void blit_bind_texture(texture * tex){
  blit_current_texture = tex;
  if(tex == NULL){
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(shader.textured_loc, 0);
  }else{
    gl_texture_bind(*tex);
    //glBindTexture(GL_TEXTURE_2D, tex->handle->tex);
    glUniform1i(shader.textured_loc, 1);
    
  }  
}
void blit_begin(BLIT_MODE _blit_mode){
  //return;
  blit_mode = _blit_mode;
  if(shader.blit_shader == 0){
    glGenBuffers(1, &quadbuffer);
    glGenBuffers(1, &quadbuffer_uvs);

    
    float quad[] = {0,0, 1,0, 0,1, 1,1};
    // triangle strip
    float quad_uvs[] = {0,0,1,0,0,1,1,1};
    
    
    glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, quadbuffer_uvs);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_uvs), quad_uvs, GL_STATIC_DRAW);

    #ifndef __EMSCRIPTEN__
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION,
                              GL_DONT_CARE,
                              GL_DONT_CARE,
                              0,
                              NULL,
                              GL_TRUE);
    //glDisable( GL_DEBUG_MESSAGE_CONTROL, GL_FALSE);
    glDebugMessageCallback( MessageCallback, 0 );
    #endif
    shader.blit_shader = gl_shader_compile2((char *)texture_shader_vs,texture_shader_vs_len, (char *)texture_shader_fs,texture_shader_fs_len);

    shader.pos_attr = 0;
    shader.tex_coord_attr = 1;
    shader.vertex_transform_loc = glGetUniformLocation(shader.blit_shader, "vertex_transform");
    shader.uv_transform_loc = glGetUniformLocation(shader.blit_shader, "uv_transform");
    shader.texture_loc = glGetUniformLocation(shader.blit_shader, "_texture");
    shader.color_loc = glGetUniformLocation(shader.blit_shader, "color");
    shader.textured_loc = glGetUniformLocation(shader.blit_shader, "textured");
    glUseProgram(shader.blit_shader);
  }
  glUseProgram(shader.blit_shader);

  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
  glVertexAttribPointer(shader.pos_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer_uvs);
  glVertexAttribPointer(shader.tex_coord_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glUniform1i(shader.textured_loc, 1);
  mat3 identity = mat3_identity();
  blit_transform = identity;
  blit_bind_texture(NULL);
  int w,h;
  if(blit_mode & BLIT_MODE_PIXEL){
    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );
    blit_scale(1.0f / (float)viewport[2], -1.0f/ (float)viewport[3]);
    w = viewport[2];
    h = viewport[3];
    if(blit_mode & BLIT_MODE_SCREEN_BIT){
      blit_translate(-w,-h);
      blit_scale(2,2);
    }
  }
  
  glUniformMatrix3fv(shader.uv_transform_loc, 1, false, &identity.m00);
  glUniform1i(shader.texture_loc, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

texture * get_default_tex(){
  static texture tex;
  if(tex.handle == NULL){
    var img = image_new(4,4,4);
    memset(image_data(&img),255, 4 * 4 * 4);
    tex = texture_from_image2(&img, TEXTURE_INTERPOLATION_NEAREST);
    image_delete(&img);
  }
  return &tex;
		 
}

void blit_uv_matrix(mat3 uv){
  blit_uv_transform = uv;
  glUniformMatrix3fv(shader.uv_transform_loc, 1, false, &blit_uv_transform.m00);
}

void blit_color(f32 r, f32 g, f32 b ,f32 a){
  _blit_color = vec4_new(r,g,b,a);
  glUniform4f(shader.color_loc, r, g, b, a);
}

void blit_quad(){
  glUniformMatrix3fv(shader.vertex_transform_loc, 1, false, &blit_transform.m00);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void blit2(texture * tex){
  glUniform4f(shader.color_loc, 1,1,1,1);
  blit_bind_texture(tex);
  blit_quad();
}



void blit(float x,float y, texture * tex){

  blit_translate(x,y);
  if(blit_mode & BLIT_MODE_PIXEL){
    blit_scale(tex->width, tex->height);
  }
  blit2(tex);
  
  if(blit_mode & BLIT_MODE_PIXEL){
    blit_scale(1.0f / tex->width, 1.0f/ tex->height);
  }
  blit_translate(-x,-y);
}

void blit_rectangle2(float r, float g, float b, float a){
  var tex = get_default_tex();
  blit_bind_texture(tex);

  glUniformMatrix3fv(shader.vertex_transform_loc, 1, false, &blit_transform.m00);
  glUniform4f(shader.color_loc, r, g, b, a);

  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}


void blit_rectangle(float x, float y, float w, float h, float r, float g, float b, float a){

  var t = blit_transform;
  blit_translate(x,y);
  blit_scale(w, h);
  blit_rectangle2(r,g,b,a);
  blit_transform = t;
}



void blit_translate(float x, float y){
  blit_transform = mat3_mul(blit_transform, mat3_2d_translation(x, y) );
  
}

void blit_scale(float x, float y){
  blit_transform = mat3_mul(blit_transform, mat3_2d_scale(x, y));
}

static blit_framebuffer * current_frame_buffer = {0};

void blit_create_framebuffer(blit_framebuffer * buf){
  if(buf->channels == 0) buf->channels = 3;

  
  blit_bind_texture(NULL);
  ASSERT(buf->width > 0 && buf->height > 0);
  ASSERT(current_frame_buffer == NULL);
  image img = {.source = NULL, .width = buf->width, .height = buf->height, .channels = buf->channels, .mode = buf->mode};
  texture tex = texture_from_image3(&img, TEXTURE_INTERPOLATION_LINEAR, TEXTURE_INTERPOLATION_NEAREST);

  glGenFramebuffers(1, &buf->id);
  glBindFramebuffer(GL_FRAMEBUFFER, buf->id); 
  gl_texture_bind(tex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.handle->tex, 0);
  //printf("ERROR: %i\n", glGetError());
  
  ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE); 
  buf->texture = tex.handle;
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void blit_use_framebuffer(blit_framebuffer * buf){
  ASSERT(current_frame_buffer == NULL);
  current_frame_buffer = buf;
  glBindFramebuffer(GL_FRAMEBUFFER, buf->id);
  ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glViewport(0, 0, buf->width, buf->height);
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

}

void blit_unuse_framebuffer(blit_framebuffer * buf){
  ASSERT(current_frame_buffer == buf);
  current_frame_buffer = NULL;
  ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int width, height;
  gl_window_get_size(current_window, &width, &height);
  glViewport(0,0, width, height);
}

texture blit_framebuffer_as_texture(blit_framebuffer * buf){
  texture tex = {.width = buf->width, .height = buf->height, .handle = buf->texture};
  return tex;
}

void blit_blit_framebuffer(blit_framebuffer * buf){
  var tex = blit_framebuffer_as_texture(buf);
  blit(0, 0, &tex);
}

