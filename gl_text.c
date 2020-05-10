
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

texture * font_tex;
#define CHAR_DATA_SIZE 100
static stbtt_bakedchar cdata[CHAR_DATA_SIZE];
void initialized_fonts();
void blit_text(const char * text){
  static int initialized = false;
  if(!initialized){
    initialized = true;
    initialized_fonts();
  }
  float x = 0;
  float y = 0;
  blit_bind_texture(font_tex);
  blit_push();
  mat3 translate = mat3_2d_translation(0, 0.5);
  mat3 negtranslate = mat3_2d_translation(0, -0.5);
  mat3 fliph = mat3_2d_scale(1.0, -1.0);
  fliph = mat3_mul(mat3_mul(translate, fliph), negtranslate);
  for(u64 i = 0; true ;){

    if(text[i] == 0) break;
    size_t l = 0;
    int codepoint = utf8_to_codepoint(text + i, &l);
    if(codepoint >= 32 && codepoint < (32 + CHAR_DATA_SIZE)){
      
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, font_tex->width, font_tex->height, codepoint-32, &x,&y,&q,1);
    
      vec2 size = vec2_new(q.x1 - q.x0, q.y1 - q.y0);
      vec2 offset = vec2_new(q.x0, 33 - q.y0 );
      mat3 m = mat3_2d_scale(q.s1 - q.s0, q.t1 - q.t0);
      m.data[2][0] = q.s0;
      m.data[2][1] = q.t0;
      blit_push();
      blit_uv_matrix(mat3_mul(m, fliph));

      blit_translate(offset.x,-q.y1);
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
    if(codepoint >= 32 && codepoint < (32 + CHAR_DATA_SIZE)){
      stbtt_aligned_quad q;
      stbtt_GetBakedQuad(cdata, font_tex->width, font_tex->height, codepoint-32, &x,&y,&q,1);
    }
  }
  return vec2_new(x, 15);
}

void initialized_fonts(){
  const char * fontfile = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"; //DejaVuSansMono
  u64 buffersize;
  void * buffer = read_file_to_buffer(fontfile, &buffersize);
  var img = image_new(1024, 1024, 1);
  img.mode = GRAY_AS_ALPHA;
 
  stbtt_BakeFontBitmap(buffer,0, 32.0, image_data(&img), img.width,img.height, 32,CHAR_DATA_SIZE, cdata);
  texture tex = texture_from_image2(&img, TEXTURE_INTERPOLATION_LINEAR);
  image_delete(&img);

  font_tex = iron_clone(&tex, sizeof(tex));
  
}
