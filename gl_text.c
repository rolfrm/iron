
//#include <GL/glew.h>
//#include <iron/full.h>
//#include <iron/gl.h>
#include "full.h"
#include "gl.h"
#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#define GL_LUMINANCE_ALPHA GL_RG
#define GL_LUMINANCE GL_RED
#define GL_COMPUTE_SHADER 1337 
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
//#include "text.shader.c"
#include "stb_truetype.h"
#include "utf8.h"

struct _font{
  stbtt_bakedchar * cdata;
  size_t cdata_count;
  texture font_tex;
  size_t font_size;
};

static font * current_font;

void blit_text(const char * text){
  if(current_font == NULL){
    ERROR("NO FONT LOADED");
    return;
  }
  
  var font_tex = &current_font->font_tex;
  var cdata = current_font->cdata;
  float x = 0;
  float y = 0;
  blit_bind_texture(font_tex);
  blit_push();
  mat3 translate = mat3_2d_translation(0, 0.5);
  mat3 negtranslate = mat3_2d_translation(0, -0.5);
  mat3 fliph = mat3_2d_scale(1.0, -1.0);
  bool screen_blit = blit_mode_get() & BLIT_MODE_SCREEN_BIT;
  if(screen_blit){
    fliph = mat3_identity();
  }else{
    fliph = mat3_mul(mat3_mul(translate, fliph), negtranslate);
  }
  glEnable(GL_BLEND);
  for(u64 i = 0; true ;){

    if(text[i] == 0) break;
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (int)(32 + current_font->cdata_count)){
      
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, font_tex->width, font_tex->height, codepoint-32, &x,&y,&q,1);
		
      vec2 size = vec2_new(q.x1 - q.x0, q.y1 - q.y0);
      mat3 m = mat3_2d_scale(q.s1 - q.s0, q.t1 - q.t0);
      m.data[2][0] = q.s0;
      m.data[2][1] = q.t0;

      blit_push();

      if(screen_blit) {
		  blit_uv_matrix(m);
		  blit_translate(q.x0, current_font->font_size + q.y0);
      } else {
		  blit_uv_matrix(mat3_mul(m, fliph));
		  blit_translate(q.x0,-q.y1);
      }
      blit_scale(size.x, size.y);
      blit_quad();
      blit_pop();
      
    }
    i += MAX((u32)l, (u32)1);
    
  }
  blit_pop();
  blit_uv_matrix(mat3_identity());
  blit_bind_texture(NULL);
}


void blit3d_text2(blit3d_context * ctx, mat4 view, mat4 model, const char * text, f32 max_width){
  if(current_font == NULL){
    ERROR("NO FONT LOADED");
    return;
  }
  
  var font_tex = &current_font->font_tex;
  var cdata = current_font->cdata;
  float x = 0;
  float y = 0;
  blit3d_bind_texture(ctx, font_tex);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for(u64 i = 0; true ;){
    
    if(text[i] == 0) break;
    if(text[i] == '\n' || x > max_width){
      y += current_font->font_size;
      x = 0;
      i += 1;
      continue;
    }
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (int)(32 + current_font->cdata_count)){
      
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, font_tex->width, font_tex->height, codepoint-32, &x,&y,&q,1);
		
      vec3 size = vec3_new(q.x1 - q.x0, q.y1 - q.y0, 0);

      mat3 m = mat3_2d_scale(q.s1 - q.s0, q.t1 - q.t0);
      m.data[2][0] = q.s0;
      m.data[2][1] = q.t0;

      blit3d_uv_matrix(ctx, m);
      mat4 t = mat4_translate((float)q.x0 ,(float) (current_font->font_size + q.y0) , 0);
      
      mat4 s = mat4_scaled(size.x, size.y, 1.0);
      var t2 = mat4_mul(view, mat4_mul(model, mat4_mul(t, s)));
      
      blit3d_view(ctx, t2);
      blit3d_blit_quad(ctx);
    }
    i += MAX((u32)l, (u32)1);
    
  }
  
  blit3d_uv_matrix(ctx, mat3_identity());
  blit3d_bind_texture(ctx, NULL);
  glDisable(GL_BLEND);
  
}

void blit3d_text(blit3d_context * ctx, mat4 view, mat4 model, const char * text){
  blit3d_text2(ctx, view, model, text, f32_infinity);
}

static vec2 measure_text(const char * text, size_t len, f32 max_width){

  float x = 0.0f;
  float x_max = 0.0f;
  float y = 0.0f;
  float y_max = 0;
  for(u64 i = 0; i < len; i++){
    if(text[i] == 0) break;
    size_t l = 0;
	 if(text[i] == '\n' || x > max_width){
		x = 0;
		y += current_font->font_size;
      y_max = MAX(y_max, y);
		continue;
	 }
    int codepoint = utf8_to_codepoint(text + i, &l);
	 
    if(codepoint >= 32 && codepoint < (int)(32 + current_font->cdata_count)){
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(current_font->cdata, current_font->font_tex.width, current_font->font_tex.height, codepoint-32, &x,&y,&q,1);
		x_max = MAX(x_max, x);
		y_max = MAX(y_max, y);
		i = i - 1 + l;
    }
  }
  return vec2_new(x_max, y_max + current_font->font_size);
}

vec2 blit_measure_text(const char * text){
  return measure_text(text, strlen(text), f32_infinity);
}

font * blit_load_font_from_buffer(void * data, float font_size){
  int char_data_count = 500;
  var img = image_new(512, 512, 1);
  img.mode = IMAGE_MODE_GRAY_AS_ALPHA;
  stbtt_bakedchar * cdata = alloc0(sizeof(cdata[0]) * char_data_count);
  stbtt_BakeFontBitmap(data, 0, font_size, image_data(&img), img.width,img.height, 32,char_data_count, cdata);
  //image_save(&img, "fonttex.png");
  texture tex = texture_from_image2(&img, TEXTURE_INTERPOLATION_LINEAR);
  image_delete(&img);
  font fnt ={.font_tex = tex, .cdata = cdata, .cdata_count = char_data_count, .font_size = font_size};
  return iron_clone(&fnt, sizeof(fnt));
}

font * blit_load_font_file(const char * fontfile, float font_size){
  size_t buffersize;
  void * buffer = read_file_to_buffer(fontfile, &buffersize);
  font * ft = blit_load_font_from_buffer(buffer, font_size);
  dealloc(buffer);
  return ft;
}

font * default_font;

void blit_set_current_font(font * fnt){
  if(fnt == NULL){
    current_font = default_font;
  }else{
    current_font = fnt;
  }
}
