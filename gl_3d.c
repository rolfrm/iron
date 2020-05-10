#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include "stb_image.h"

extern unsigned char texture_3d_shader_vs[];
extern unsigned int texture_3d_shader_vs_len;

extern unsigned char texture_shader_fs[];
extern unsigned int texture_shader_fs_len;

typedef struct _shader_3d{
  int vertex_transform_loc, uv_transform_loc, texture_loc, textured_loc, color_loc;
  int pos_attr, tex_coord_attr;
  int blit_shader;
  
}shader_3d;

struct _blit3d_context{
  shader_3d shader;
  bool initialized;
  mat4 matrix;  
  vec4 color;
};

blit3d_context * blit3d_context_new(){
  blit3d_context * ctx = alloc0(sizeof(ctx[0]));
  blit3d_view(ctx, mat4_identity());
  return ctx;

}

void blit3d_context_initialize(blit3d_context * ctx){
  ctx->initialized = true;
  shader_3d shader;
  shader.blit_shader = gl_shader_compile2((char *)texture_3d_shader_vs,texture_3d_shader_vs_len, (char *)texture_shader_fs,texture_shader_fs_len);
  shader.pos_attr = 0;
  shader.tex_coord_attr = 1;
  shader.vertex_transform_loc = glGetUniformLocation(shader.blit_shader, "vertex_transform");
  shader.uv_transform_loc = glGetUniformLocation(shader.blit_shader, "uv_transform");
  shader.texture_loc = glGetUniformLocation(shader.blit_shader, "_texture");
  shader.color_loc = glGetUniformLocation(shader.blit_shader, "color");
  shader.textured_loc = glGetUniformLocation(shader.blit_shader, "textured");
  glUseProgram(shader.blit_shader);
  
  ctx->shader = shader;
  
}

void blit3d_context_load(blit3d_context * ctx)
{
  if(false == ctx->initialized)
    blit3d_context_initialize(ctx);

  glUseProgram(ctx->shader.blit_shader);
  glEnableVertexAttribArray(0);
  //mglEnableVertexAttribArray(1);
}

void blit3d_view(blit3d_context * ctx, mat4 viewmatrix){

  ctx->matrix = viewmatrix;
}

struct _blit3d_polygon{
  void * data;
  size_t length;

  u32 dimensions;
  u32 buffer;
  u32 element_buffer;
  bool changed;
};

blit3d_polygon * blit3d_polygon_new(){
  blit3d_polygon * pol = alloc0(sizeof(pol[0]));
  return pol;
}

void blit3d_polygon_load_data(blit3d_polygon * polygon, void * data, size_t size){
  polygon->data = iron_clone(data, size);
  polygon->length = size;
  polygon->changed = true;
}

void blit3d_polygon_configure(blit3d_polygon * polygon, int dimensions){
  polygon->dimensions = dimensions;
}

void blit3d_polygon_update(blit3d_polygon * polygon){
  if(polygon->changed){
    polygon->changed = false;
    if(polygon->buffer == 0){
      glGenBuffers(1, &polygon->buffer);
      glGenBuffers(1, &polygon->element_buffer);

    }

    glBindBuffer(GL_ARRAY_BUFFER, polygon->buffer);
    glBufferData(GL_ARRAY_BUFFER, polygon->length, polygon->data, GL_STATIC_DRAW);

    u8 * elements = alloc0(polygon->length);
    for(u8 i = 0; i < polygon->length / 12; i++)
      elements[i] = i;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygon->element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, polygon->length / 12,elements, GL_STATIC_DRAW);    

    free(polygon->data);
    free(elements);
  }
}
struct _texture_handle {
  GLuint tex;
};
texture * get_default_tex();
void blit3d_polygon_blit(blit3d_context * ctx, blit3d_polygon * polygon){
  //  return;

  var tex = get_default_tex();
  glBindTexture(GL_TEXTURE_2D, tex->handle->tex);

  blit3d_polygon_update(polygon);


  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygon->element_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, polygon->buffer);
  var shader = ctx->shader;
  var c = ctx->color;
  glUniform4f(shader.color_loc, c.x,c.y,c.z,c.w);
  
  glVertexAttribPointer(shader.pos_attr, polygon->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  
  //glVertexAttribPointer(shader.tex_coord_attr, polygon->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  glUniformMatrix4fv(shader.vertex_transform_loc, 1, false, &ctx->matrix.m00);
  //printf("eRR1: %i %i %i %i\n", glGetError(), polygon->length, polygon->dimensions, polygon->length / polygon->dimensions / 4);
  glDrawElements(GL_TRIANGLE_STRIP,polygon->length / (polygon->dimensions * 4), GL_UNSIGNED_BYTE, 0);
  //printf("eRR2: %i\n", glGetError());
  
}

void blit3d_color(blit3d_context * ctx, vec4 color){
  ctx->color = color;
}
