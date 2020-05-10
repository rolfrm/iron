
//#include <GL/glew.h>
//#include <iron/full.h>
//#include <iron/gl.h>
#include "full.h"
#include "gl.h"
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
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

void initialized_fonts();
void blit_text(const char * text){
  static int initialized = false;
  if(!initialized){
    initialized = true;
    initialized_fonts();
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
  bool screen_blit = blit_mode_get() | BLIT_MODE_SCREEN_BIT;
  if(screen_blit){
    fliph = mat3_identity();
  }else{
    fliph = mat3_mul(mat3_mul(translate, fliph), negtranslate);
  }
  for(u64 i = 0; true ;){

    if(text[i] == 0) break;
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (32 + current_font->cdata_count)){
      
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
  //glDisable(GL_BLEND);
}


vec2 measure_text(const char * text, size_t len){
  float x = 0;
  float y = 0;
  for(u64 i = 0; i < len; i++){
    if(text[i] == 0) break;
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (32 + current_font->cdata_count)){
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(current_font->cdata, current_font->font_tex.width, current_font->font_tex.height, codepoint-32, &x,&y,&q,1);
    }
  }
  return vec2_new(x, current_font->font_size);
}


font * blit_load_font_from_buffer(void * data, size_t size, float font_size){
  int char_data_count = 100;
  var img = image_new(1024, 1024, 1);
  img.mode = GRAY_AS_ALPHA;
  stbtt_bakedchar * cdata = alloc0(sizeof(cdata[0]) * char_data_count);
  stbtt_BakeFontBitmap(data,0, font_size, image_data(&img), img.width,img.height, 32,char_data_count, cdata);
  texture tex = texture_from_image2(&img, TEXTURE_INTERPOLATION_LINEAR);
  image_delete(&img);
  font fnt ={.font_tex = tex, .cdata = cdata, .cdata_count = char_data_count, .font_size = font_size};
  return iron_clone(&fnt, sizeof(fnt));
}

font * blit_load_font_file(const char * fontfile, float font_size){
  u64 buffersize;
  void * buffer = read_file_to_buffer(fontfile, &buffersize);
  return blit_load_font_from_buffer(buffer, buffersize, font_size);
}

font * default_font;

void blit_set_current_font(font * fnt){
  if(fnt == NULL){
    current_font = default_font;
  }else{
    current_font = fnt;
  }
}

void initialized_fonts(){
  const char * fontfile = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"; //DejaVuSansMono
  if(default_font == NULL){
    default_font = blit_load_font_file(fontfile,  20);
    if(current_font == NULL)
      current_font = default_font;
  }
}
