#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include "stb_image.h"
#include "texture.shader.c"
#include <signal.h>
#ifdef _EMCC_
#include <emscripten.h>

EM_JS(int, canvas_get_width, (), {
  return emscripten_border.clientWidth;
});

EM_JS(int, canvas_get_height, (), {
    return emscripten_border.clientHeight;
  });

void gl_canvas_get_size(int * w, int * h){

  w[0] = canvas_get_width();
  h[0] = canvas_get_height();
}

#else
void gl_canvas_get_size(int * w, int * h){
  UNUSED(w);
  UNUSED(h);
  // not implemented
}
#endif


struct _gl_window{
  void * handle;
};
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
#ifndef __EMSCRIPTEN__
	
    //glEnable( GL_DEBUG_OUTPUT );
    /*glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION,
                              GL_DONT_CARE,
                              GL_DONT_CARE,
                              0,
                              NULL,
                              GL_TRUE);*/
    //glDisable( GL_DEBUG_MESSAGE_CONTROL, GL_FALSE);
    //glDebugMessageCallback( MessageCallback, 0 );
    #endif
    
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

void gl_window_show_cursor(gl_window * win, iron_cursor_type mode){
  var f = current_backend->show_cursor;
  if(f == NULL) return;
  f(win->handle, mode);
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
  
  image img = {.width = width, .height = height, .channels = channels, .source = IRON_CLONE(src), .mode = IMAGE_MODE_NONE};
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
  return image_new2(width, height, channels, IMAGE_MODE_NONE);
}
image image_new2(int width, int height, int channels, image_mode mode){

  image_source src;
  src.kind = IMAGE_SOURCE_MEMORY;

  size_t pixel_size = 1;
  if(mode & IMAGE_MODE_F32)
    pixel_size = 4;
  
  src.data = alloc0(width * height * channels * pixel_size);

  image img = {.width = width, .height = height, .channels = channels, .source = IRON_CLONE(src), .mode = mode};
  return img;
}

image image_new3(int width, int height, int depth, int channels, image_mode mode){

  image_source src;
  src.kind = IMAGE_SOURCE_MEMORY;

  size_t pixel_size = 1;
  if(mode & IMAGE_MODE_F32)
    pixel_size = 4;
  
  src.data = alloc0(width * height * depth * channels * pixel_size);

  image img = {.width = width, .height = height, .depth = depth, .is_3d = true, .channels = channels, .source = IRON_CLONE(src), .mode = mode};
  return img;
}


void image_delete(image * img){
  if(img->source->data != NULL)
    dealloc(img->source->data);
  dealloc(img->source);
  image im2 = {0};
  img[0] = im2;
}

void image_fill(image img, u32 color){
  int stride = img.channels;
  int w = img.width, h = img.height;
  void * data = image_data(&img);
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      u8 * p = (j + i * w) * stride + data;
      memcpy(p, &color, stride);
    }
  }
}

static float _len(f32 a, f32 b){
  return sqrtf(a * a + b * b);
}

u64 image_compare(image img, image img2){
  int stride = img.channels;
  int w = img.width, h = img.height;
  if(img.height != img2.height || img.width != img2.width)
    return (u64)_len((img.height - img2.height), (img.width - img2.width));
  if(img.channels != img2.channels)
    return 0xFFFFFFF;
  u32 c = 0;
  void * data = image_data(&img);
  void * data2 = image_data(&img2);
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      u8 * p = (j + i * w) * stride + data;
      u8 * p2 = (j + i * w) * stride + data2;
      int diff = 0;
      for(int k = 0; k < stride; k++){
	u8 d = p[k] - p2[k];
	if(d > 128)
	  d = p2[k] - p[k];
	if(d == 0){

	}else if (d < 10){
	  d = 1;
	}else if (d < 100){
	  d = 2;
	}else {
	  d = 3;
	}
	diff = MAX(diff, d);
      }
      c += diff;   
    }
  }
  return c;
}

f64 image_comparef (image img, image img2){
  int w = img.width, h = img.height;
  u64 c = w * h;
  return image_compare(img, img2) / (f64)c;
}


image image_diff (image img, image img2){
  int w = img.width, h = img.height;
  u64 c = w * h * img.channels;
  u8 * data = image_data(&img);
  u8 * data2 = image_data(&img2);
  image img3 = image_new(w, h, img.channels);
  u8 * data3 = image_data(&img3);
  for(u64 i = 0 ;i < c; i++){
    for(int j = 0; j < 3; j++,i++){
      u8 x = data[i];
      u8 y = data2[i];
      u8 z;
      if(x > y)
	z = y - x;
      else
	z = x - y;
      data3[i] = z;
    }
    data3[i] = 255;
  }
  return img3;
}


struct _texture_handle {
  GLuint tex;
  int format;
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
texture texture_new(TEXTURE_INTERPOLATION sub_interp, TEXTURE_INTERPOLATION super_interp, bool is_3d){

  GLuint tex;
  var type = is_3d ? GL_TEXTURE_3D : GL_TEXTURE_2D;
  glGenTextures(1, &tex);
  glBindTexture(type, tex);  

  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl_tex_interp(sub_interp, false));
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl_tex_interp(super_interp, true));


  glTexParameteri(type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  texture_handle hndl = {.tex = tex};
  texture texture = { .handle = IRON_CLONE(hndl), .width = 0, .height = 0, .depth = 0, .is_3d = is_3d};
  return texture;
}

static int to_f32_enum(int enum_in){
  switch(enum_in){
  case GL_RGBA: return GL_RGBA32F;
  case GL_RGB: return GL_RGB32F;
  case GL_LUMINANCE_ALPHA:
  case GL_RG: return GL_RG32F;
  case GL_LUMINANCE:
  case GL_R: return GL_R32F;
  default:
    ERROR("Unsupported format!");
    return 0;
  }
}

void texture_load_image(texture * texture, image * image){
  gl_texture_bind(*texture);
  int chn[] = {GL_RED, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
  int int_formats[] ={GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
  int int_format = int_formats[image->channels - 1];
  int format = chn[image->channels - 1];
  int type = GL_UNSIGNED_BYTE;
  u8 * data = image_data(image);
  bool delete_data = false;
  if(image->mode & IMAGE_MODE_GRAY_AS_ALPHA && image->channels == 1){
    format = GL_LUMINANCE_ALPHA;
    int_format = GL_LUMINANCE_ALPHA;
    // cannot do swizzle as it is not supported in webgl.
    // instead provide a temporary LUMIANCE_ALPHA texture.
    if(data != NULL){
      int pixels = image->width * image->height * (image->is_3d ? image->depth : 1);
      u8 * newdata = alloc0(2 * pixels);
      for(int i = 0; i < pixels; i++){
		newdata[i * 2] = 0xFF;
		newdata[i * 2 + 1] = data[i];
      }
      data = newdata;
      delete_data = true;
    }
  }
  
  if(image->mode == IMAGE_MODE_DEPTH16){
	int_format = GL_DEPTH_COMPONENT32F;
	format = GL_DEPTH_COMPONENT;
	type = GL_FLOAT;
	printf("LOAD DEPTH MODE IMAGE\n");
  }
  
  if(image->mode == IMAGE_MODE_F32){
    int_format = to_f32_enum(int_formats[image->channels - 1]);
    type = GL_FLOAT;
  }

  if(image->is_3d){

	 glTexImage3D(GL_TEXTURE_3D, 0, int_format,
						 image->width, image->height, image->depth, 0, format, type, data);
  }else{
	 
	 glTexImage2D(GL_TEXTURE_2D, 0, int_format,
					  image->width, image->height, 0, format, type, data);
	 
  }
  
  texture->width = image->width;
  texture->height = image->height;
  texture->depth = image->is_3d ? image->depth : 1 ;
  texture->is_3d = image->is_3d;
  texture->handle->format = int_format;
  if(delete_data)
    dealloc(data);
}

texture texture_from_image3(image * image, TEXTURE_INTERPOLATION sub_interp, TEXTURE_INTERPOLATION super_interp){
  var texture = texture_new(sub_interp, super_interp, image->is_3d);
  texture_load_image(&texture, image);
  if(sub_interp == TEXTURE_INTERPOLATION_BILINEAR || super_interp == TEXTURE_INTERPOLATION_BILINEAR){
	 if (texture.is_3d){
		glGenerateMipmap(GL_TEXTURE_3D);
	 }else{
		glGenerateMipmap(GL_TEXTURE_2D);
	 }
  }
  return texture;
}

texture texture_from_image2(image * image, TEXTURE_INTERPOLATION interp){
  return texture_from_image3(image, interp, interp);
}

texture texture_from_image(image * image){
  return texture_from_image2(image, TEXTURE_INTERPOLATION_BILINEAR);
}

void texture_to_image(texture * tex, image * image){
  if(image->height != tex->height || image->width != tex->width){
    ERROR("Texture and image formats does not match");
    return;
  }
  u8 * data = image_data(image);
  if(data == NULL){
    ERROR("Image data not allocated");
    return;
  }
  
  int format[] ={GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};
  

  if(image->mode & IMAGE_MODE_GRAY_AS_ALPHA && image->channels == 1){
    format[0] = GL_LUMINANCE_ALPHA;
  }
  int typesize = 1;
  int type = GL_UNSIGNED_BYTE;
  if(image->mode & IMAGE_MODE_F32){
    //format[image->channels - 1] = to_f32_enum(format[image->channels - 1]);
    type = GL_FLOAT;
    typesize = 4;
  }
  int buffer_size = image->height * image->width * image->channels * typesize;
  glGetTextureImage(tex->handle->tex, 0,  format[image->channels - 1], type, buffer_size, data);
}

void gl_texture_bind(texture tex){
  glBindTexture(tex.is_3d ? GL_TEXTURE_3D : GL_TEXTURE_2D, tex.handle->tex);
}

void gl_texture_image_bind(texture tex, int channel, texture_bind_options options){

  int access = 0;
  if(options == TEXTURE_BIND_READ_WRITE){
    access = GL_READ_WRITE;
  }else if(options == TEXTURE_BIND_READ){
    access = GL_READ_ONLY;
  }else if(options == TEXTURE_BIND_WRITE){
    access = GL_WRITE_ONLY;
  }

  glBindImageTexture(channel, tex.handle->tex, 0, GL_FALSE, 0, access, GL_RGBA8);
}

u32 gl_texture_handle(texture tex){
  return tex.handle->tex;
}


u32 compile_shader(int shader_type, const char * code){
  u32 ss = glCreateShader(shader_type);
  i32 l = strlen(code);
  glShaderSource(ss, 1, (void *) &code, &l); 
  glCompileShader(ss);
  int compileStatus = 0;
  printf("Compiling shader: %s\n", code);
  glGetShaderiv(ss, GL_COMPILE_STATUS, &compileStatus);
  if(compileStatus == GL_FALSE){
    logd("Error during shader compilation:\n");
    int loglen = 0;
    glGetShaderiv(ss, GL_INFO_LOG_LENGTH, &loglen);
    char * buffer = alloc0(loglen + 10);
    glGetShaderInfoLog(ss, loglen, &loglen, buffer);
    buffer[loglen] = 0;
    printf("%i: '%s'\n", loglen, buffer);
	 printf("**** code ****\n: %s\n", code);
    dealloc(buffer);
	 ASSERT(false);
  } else{
	int loglen = 0;
    glGetShaderiv(ss, GL_INFO_LOG_LENGTH, &loglen);
	if(loglen > 0){
	  char * buffer = alloc0(loglen + 10);
	  glGetShaderInfoLog(ss, loglen, &loglen, buffer);
	  buffer[loglen] = 0;
	  printf("%i: '%s'\n", loglen, buffer);
	  printf("**** code ****\n: %s\n", code);
	  dealloc(buffer);
	}
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
  u32 program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  
  glLinkProgram(program);

  // Check for errors
  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    // An error occurred during linking, get the log message
    GLint logLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    GLchar logMessage[logLength];
    glGetProgramInfoLog(program, logLength, NULL, logMessage);
    printf("OpenGL error: %s\n", logMessage);
  }
  
  return program;
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

u32 gl_compile_compute_shader(const char * compute_shader, int compute_shader_len){
  void * buffer = alloc0(compute_shader_len + 1);
  memcpy(buffer, compute_shader, compute_shader_len);
  u32 program = glCreateProgram();
  u32 vs = compile_shader(GL_COMPUTE_SHADER, buffer);
  glAttachShader(program, vs);
  glLinkProgram(program);
  dealloc(buffer);
  return program;
}

u32 create_shader_from_files(const char * vs_path, const char * fs_path){
  char * vscode = read_file_to_string(vs_path);
  char * fscode = read_file_to_string(fs_path);
  u32 prog = gl_shader_compile(vscode, fscode);
  dealloc(vscode);
  dealloc(fscode);
  return prog;
}


typedef struct _textured_shader{
  int vertex_transform_loc, uv_transform_loc, texture_loc, color_loc;
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
  }else{
    gl_texture_bind(*tex);    
  }  
}
void blit_begin(BLIT_MODE _blit_mode){

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

    shader.blit_shader = gl_shader_compile2((char *)texture_shader_vs,texture_shader_vs_len, (char *)texture_shader_fs,texture_shader_fs_len);

    shader.pos_attr = 0;
    shader.tex_coord_attr = 1;
    shader.vertex_transform_loc = glGetUniformLocation(shader.blit_shader, "vertex_transform");
    shader.uv_transform_loc = glGetUniformLocation(shader.blit_shader, "uv_transform");
    shader.texture_loc = glGetUniformLocation(shader.blit_shader, "_texture");
    shader.color_loc = glGetUniformLocation(shader.blit_shader, "color");

  }
  glUseProgram(shader.blit_shader);
  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
  glVertexAttribPointer(shader.pos_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer_uvs);
  glVertexAttribPointer(shader.tex_coord_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);

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
  blit_uv_matrix(mat3_identity());
  
  glUniformMatrix3fv(shader.uv_transform_loc, 1, false, &identity.m00);
  glUniform1i(shader.texture_loc, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  //glBindBuffer(GL_ARRAY_BUFFER, 0);
}

texture * get_default_tex(void){
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
  if(blit_current_texture == NULL){
	 var tex = get_default_tex();
	 blit_bind_texture(tex);
  }
  glUniformMatrix3fv(shader.vertex_transform_loc, 1, false, &blit_transform.m00);
  glUniformMatrix3fv(shader.uv_transform_loc, 1, false, &blit_uv_transform.m00);
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
 
  glUniform4f(shader.color_loc, r, g, b, a);

  blit_quad();
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

  image dimg = {.source = NULL, .width = buf->width, .height = buf->height, .channels = 1, .mode = buf->depth_mode};
  
  texture tex = texture_from_image3(&img, TEXTURE_INTERPOLATION_LINEAR, TEXTURE_INTERPOLATION_NEAREST);


  
  glGenFramebuffers(1, &buf->id);
  glBindFramebuffer(GL_FRAMEBUFFER, buf->id);

  if(dimg.mode != IMAGE_MODE_NONE){
	var dtex = texture_from_image3(&dimg, TEXTURE_INTERPOLATION_LINEAR, TEXTURE_INTERPOLATION_NEAREST);
	gl_texture_bind(dtex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dtex.handle->tex, 0);
	buf->depth_texture = dtex.handle;

  }
  
  gl_texture_bind(tex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.handle->tex, 0);
  
  
  
  //printf("ERROR: %i\n", glGetError());
  printf("FRAMEBUFFER OK?? %i %i \n", glGetError(), glCheckFramebufferStatus(GL_FRAMEBUFFER));
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
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
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

texture blit_framebuffer_depth_as_texture(blit_framebuffer * buf){
  texture tex = {.width = buf->width, .height = buf->height, .handle = buf->depth_texture};
  return tex;
}


void blit_blit_framebuffer(blit_framebuffer * buf){
  var tex = blit_framebuffer_as_texture(buf);
  blit(0, 0, &tex);
}

void blit_clear(){
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void blit_delete_framebuffer(blit_framebuffer * buf){
  glDeleteFramebuffers(1, &buf->id);
}
