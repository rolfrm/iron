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
  texture * current_texture;
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
  glEnableVertexAttribArray(shader.pos_attr);
  glEnableVertexAttribArray(shader.tex_coord_attr);
  ctx->shader = shader;  
}

void blit3d_context_load(blit3d_context * ctx)
{
  if(false == ctx->initialized)
    blit3d_context_initialize(ctx);

  glUseProgram(ctx->shader.blit_shader);
  glEnableVertexAttribArray(0);
}

void blit3d_view(blit3d_context * ctx, mat4 viewmatrix){

  ctx->matrix = viewmatrix;
}

struct _blit3d_polygon{
  void * data;
  size_t length;
  u32 dimensions;
  u32 buffer;
  bool changed;
  vertex_buffer_type type;
};

blit3d_polygon * blit3d_polygon_new(){
  blit3d_polygon * pol = alloc0(sizeof(pol[0]));
  pol->type = VERTEX_BUFFER_ARRAY;
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
     }
    if(polygon->type == VERTEX_BUFFER_ARRAY){
      glBindBuffer(GL_ARRAY_BUFFER, polygon->buffer);
      glBufferData(GL_ARRAY_BUFFER, polygon->length, polygon->data, GL_STATIC_DRAW);
    }
    else if(polygon->type == VERTEX_BUFFER_ELEMENTS){
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygon->buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, polygon->length, polygon->data, GL_STATIC_DRAW);
    }
    free(polygon->data);
    polygon->data = NULL;
  }
}
struct _texture_handle {
  GLuint tex;
};
texture * get_default_tex();

void blit3d_polygon_blit(blit3d_context * ctx, blit3d_polygon * polygon){
  var tex = get_default_tex();
  glBindTexture(GL_TEXTURE_2D, tex->handle->tex);

  blit3d_polygon_update(polygon);

  glBindBuffer(GL_ARRAY_BUFFER, polygon->buffer);
  var shader = ctx->shader;
  var c = ctx->color;
  glUniform4f(shader.color_loc, c.x,c.y,c.z,c.w);
  
  glVertexAttribPointer(shader.pos_attr, polygon->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  
  glUniformMatrix4fv(shader.vertex_transform_loc, 1, false, &ctx->matrix.m00);
  glDrawArrays(GL_TRIANGLE_STRIP,0, polygon->length / (polygon->dimensions * 4));
}

void blit3d_polygon_blit2(blit3d_context * ctx, vertex_buffer ** polygons, u32 count){
  if(count == 0)
    return;

  var tex = ctx->current_texture != NULL ? ctx->current_texture : get_default_tex();
  glUniform1i(ctx->shader.textured_loc, 1);
  glBindTexture(GL_TEXTURE_2D, tex->handle->tex);
  int elements_index = -1;
  for(u32 i = 0; i < count; i++){
    blit3d_polygon_update(polygons[i]);
    if(polygons[i]->type == VERTEX_BUFFER_ELEMENTS){
      elements_index = (int)i; 
    }
  }

  if(elements_index != -1)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygons[elements_index]->buffer);
  
  u32 j = 0;
  for(u32 i = 0; i < count; i++){
    if((int)i == elements_index)
      continue;

    glBindBuffer(GL_ARRAY_BUFFER, polygons[j]->buffer);
    glVertexAttribPointer(j, polygons[j]->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    j += 1;    
  }

  var shader = ctx->shader;
  var c = ctx->color;
  glUniform4f(shader.color_loc, c.x,c.y,c.z,c.w);
  glUniformMatrix4fv(shader.vertex_transform_loc, 1, false, &ctx->matrix.m00);
  if(elements_index != -1){
    glDrawElements(GL_TRIANGLE_STRIP, polygons[elements_index]->length / (polygons[elements_index]->dimensions * 4), GL_UNSIGNED_BYTE, 0);

    // draw elements here
  }else{
    glDrawArrays(GL_TRIANGLE_STRIP,0, polygons[0]->length / (polygons[0]->dimensions * 4));
  }
  int err =  glGetError();
  if(err != 0)
    printf("eRR2: %i\n", err);
}


void blit3d_color(blit3d_context * ctx, vec4 color){
  ctx->color = color;
}

void blit3d_bind_texture(blit3d_context * ctx, texture * tex){
  ctx->current_texture = tex;
}
