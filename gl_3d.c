#include "full.h"
#define GL_GLEXT_PROTOTYPES
#include "gl.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include "stb_image.h"

extern unsigned char texture_3d_shader_vs[];
extern unsigned int texture_3d_shader_vs_len;

extern unsigned char texture_3d_color_shader_vs[];
extern unsigned int texture_3d_color_shader_vs_len;

extern unsigned char texture_shader_fs[];
extern unsigned int texture_shader_fs_len;

extern unsigned char texture_depth_shader_fs[];
extern unsigned int texture_depth_shader_fs_len;

extern unsigned char voxel_depth_fs[];
extern unsigned int voxel_depth_fs_len;

extern unsigned char voxel_depth2_fs[];
extern unsigned int voxel_depth2_fs_len;

extern unsigned char voxel_depth_vs[];
extern unsigned int voxel_depth_vs_len;


typedef struct _shader_3d{
  int vertex_transform_loc, uv_transform_loc, texture_loc, textured_loc, color_loc;
  int pos_attr, tex_coord_attr,color_attr;
  int blit_shader;
  
}shader_3d;

typedef struct {
  u32 program;
  u32 program2;
  int pos_attr;
  int vertex_transform_loc;
  int camera_position_loc;
  int voxel_offset_loc;
  int voxel_scale_loc;
}voxel_depth_shader;

struct _blit3d_context{
  shader_3d shader;
  shader_3d shader2;
  shader_3d shader3;
  voxel_depth_shader voxel_depth_shader;
  bool initialized;
  mat4 matrix;
  mat3 uv_matrix;
  vec4 color;
  texture * current_texture[5];
  size_t texture_count;
  blit3d_polygon * quad_polygon;
  blit3d_mode mode;

  // voxel shader
  vec3 camera_position;
  vec3 voxel_offset;
  vec3 voxel_scale;
  
  
};

blit3d_context * blit3d_context_new(){
  blit3d_context * ctx = alloc0(sizeof(ctx[0]));
  blit3d_view(ctx, mat4_identity());
  ctx->texture_count = 1;
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
  //glEnableVertexAttribArray(shader.pos_attr);
  //glEnableVertexAttribArray(shader.tex_coord_attr);
  ctx->shader = shader;

  shader_3d shader2;
  shader2.blit_shader = gl_shader_compile2(
										   (char *)texture_3d_color_shader_vs,
										   texture_3d_color_shader_vs_len,
										   (char *)texture_shader_fs,
										   texture_shader_fs_len);
  shader2.pos_attr = 0;
  shader2.color_attr = 1;
  shader2.vertex_transform_loc = glGetUniformLocation(shader2.blit_shader, "vertex_transform");
  shader2.color_loc = glGetUniformLocation(shader2.blit_shader, "color");
  glUseProgram(shader2.blit_shader);
  //glEnableVertexAttribArray(shader.pos_attr);
  //glEnableVertexAttribArray(shader.tex_coord_attr);
  ctx->shader2 = shader2;  

  shader_3d shader3;
  shader3.blit_shader = gl_shader_compile2((char *)
										   texture_3d_shader_vs,
										   texture_3d_shader_vs_len, (char *)
										   texture_depth_shader_fs,
										   texture_depth_shader_fs_len);
  
  glUseProgram(shader3.blit_shader);
  shader3.pos_attr = 0;
  shader3.color_attr = 1;
  shader3.vertex_transform_loc = glGetUniformLocation(shader3.blit_shader, "vertex_transform");
  shader3.color_loc = glGetUniformLocation(shader3.blit_shader, "color");
  shader3.uv_transform_loc = glGetUniformLocation(shader3.blit_shader, "uv_transform");
  glUniform1i(glGetUniformLocation(shader3.blit_shader, "_texture"), 0);
  glUniform1i(glGetUniformLocation(shader3.blit_shader, "_depthtexture"), 1);
  //glEnableVertexAttribArray(shader.pos_attr);
  //glEnableVertexAttribArray(shader.tex_coord_attr);
  ctx->shader3 = shader3;  
  

  voxel_depth_shader voxel_shader = {0};
  voxel_shader.program = gl_shader_compile2((char *) voxel_depth_vs,
														  voxel_depth_vs_len,
														  (char *)voxel_depth_fs,
														  voxel_depth_fs_len);

  voxel_shader.program2 = gl_shader_compile2((char *) voxel_depth_vs,
														  voxel_depth_vs_len,
														  (char *)voxel_depth2_fs,
														  voxel_depth2_fs_len);


  glUseProgram(voxel_shader.program);
  
  voxel_shader.vertex_transform_loc = glGetUniformLocation(voxel_shader.program, "vertex_transform");
  voxel_shader.camera_position_loc = glGetUniformLocation(voxel_shader.program, "camera_position");
  voxel_shader.voxel_offset_loc = glGetUniformLocation(voxel_shader.program, "voxel_offset");
  voxel_shader.voxel_scale_loc = glGetUniformLocation(voxel_shader.program, "voxel_scale");
  voxel_shader.pos_attr = 0;
  
  
  glUniform1i(glGetUniformLocation(voxel_shader.program, "voxel_depth"), 0);
  glUniform1i(glGetUniformLocation(voxel_shader.program, "color"), 1);

  glUseProgram(voxel_shader.program2);
  glUniform1i(glGetUniformLocation(voxel_shader.program2, "voxel_depth"), 0);
  glUniform1i(glGetUniformLocation(voxel_shader.program2, "color"), 1);
  

  ctx->voxel_depth_shader = voxel_shader;
  ctx->voxel_scale = vec3_new(1,1,1);
  glUseProgram(shader.blit_shader);

}

void blit3d_set_camera_position(blit3d_context * ctx, vec3 camera_position){
  ctx->camera_position = camera_position;
}

void blit3d_set_voxel_transform(blit3d_context * ctx, vec3 offset, vec3 scale){
  ctx->voxel_offset = offset;
  ctx->voxel_scale = scale;
}


void blit3d_context_load(blit3d_context * ctx)
{
  if(false == ctx->initialized)
    blit3d_context_initialize(ctx);
  
  glUseProgram(ctx->shader.blit_shader);
  glEnableVertexAttribArray(0);
  ctx->uv_matrix = mat3_identity();
}


void blit3d_set_mode(blit3d_context * ctx, blit3d_mode mode){
  int shader = -1;
  ctx->mode = mode;
  
  if(ctx->mode == BLIT3D_TRIANGLE_STRIP ||
	  ctx->mode == BLIT3D_POINTS ||
	  ctx->mode == BLIT3D_TRIANGLES ||
	  ctx->mode == BLIT3D_TRIANGLES_COLOR ||
	  ctx->mode == BLIT3D_TRIANGLE_STRIP_COLOR){
	shader = ctx->shader.blit_shader;
  }
  if(ctx->mode == BLIT3D_TRIANGLES_COLOR || ctx->mode == BLIT3D_TRIANGLE_STRIP_COLOR){
	shader = ctx->shader2.blit_shader;
  }
  if(ctx->mode == BLIT3D_TRIANGLE_STRIP_TEXTURE_DEPTH)
	shader = ctx->shader3.blit_shader;
  if(ctx->mode == BLIT3D_VOXEL_DEPTH){
	 shader = ctx->voxel_depth_shader.program;
  }
  if(ctx->mode == BLIT3D_VOXEL_DEPTH2){
	 shader = ctx->voxel_depth_shader.program2;
  }
  if(shader != -1)
	glUseProgram(shader);
  ASSERT(shader != -1);
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

f32 * blit3d_polygon_get_verts(blit3d_polygon * polygon, u32 * len){
  if(polygon->dimensions != 3 || polygon->type != VERTEX_BUFFER_ARRAY || polygon->data == NULL)
    return NULL;
  *len = polygon->length/ 3;
  return polygon->data;
}

void blit3d_polygon_destroy(blit3d_polygon ** polygon){
  var p = *polygon;
  if(p->data != NULL)
    free(p->data);
  if(p->buffer != 0)
    glDeleteBuffers(1, &p->buffer);
  free(p);
  *polygon = NULL;
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
  if(polygon->dimensions == 0){
	 ERROR("Unconfigued polygon\n");
	 
  }  
  if(polygon->changed){
    polygon->changed = false;
    if(polygon->buffer == 0){
      glGenBuffers(1, &polygon->buffer);
     }
    if(polygon->type == VERTEX_BUFFER_ARRAY){
      glBindBuffer(GL_ARRAY_BUFFER, polygon->buffer);
      glBufferData(GL_ARRAY_BUFFER, polygon->length, polygon->data, GL_DYNAMIC_DRAW);
    }
    else if(polygon->type == VERTEX_BUFFER_ELEMENTS){
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, polygon->buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, polygon->length, polygon->data, GL_DYNAMIC_DRAW);
    }
    free(polygon->data);
    polygon->data = NULL;
  }
}
struct _texture_handle {
  GLuint tex;
};
texture * get_default_tex(void);

void blit3d_polygon_blit(blit3d_context * ctx, blit3d_polygon * polygon){
  var tex = get_default_tex();
  glActiveTexture(GL_TEXTURE0);
  
  glBindTexture(GL_TEXTURE_2D, tex->handle->tex);

  blit3d_polygon_update(polygon);

  glBindBuffer(GL_ARRAY_BUFFER, polygon->buffer);

  if(ctx->mode == BLIT3D_VOXEL_DEPTH || ctx->mode == BLIT3D_VOXEL_DEPTH2){

	 for(size_t i = 0; i < ctx->texture_count; i++){
		var tex = ctx->current_texture[i];
		
		if(tex == NULL && i == 0)
		  tex = get_default_tex();
		
		if(tex != NULL){
		  glActiveTexture(GL_TEXTURE0 + i);
		  if(tex->is_3d)
			 glBindTexture(GL_TEXTURE_3D, tex->handle->tex);
		  else
			 glBindTexture(GL_TEXTURE_2D, tex->handle->tex);
		 
		}
	 }

	 
	 var shader = ctx->voxel_depth_shader;

	 glVertexAttribPointer(shader.pos_attr, polygon->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
	 glUniformMatrix4fv(shader.vertex_transform_loc, 1, false, &ctx->matrix.m00);
	 
	 var cam = ctx->camera_position;
	 var vo = ctx->voxel_offset;
	 var vs = ctx->voxel_scale;
	 glUniform3f(shader.camera_position_loc, cam.x,cam.y,cam.z);
	 glUniform3f(shader.voxel_offset_loc, vo.x, vo.y, vo.z);
	 glUniform3f(shader.voxel_scale_loc, vs.x, vs.y, vs.z);
	 glDrawArrays(GL_TRIANGLES, 0, polygon->length / (polygon->dimensions * 4));
  

	 return;
  }
  
  var shader = ctx->shader;
  var c = ctx->color;
  
  glUniform4f(shader.color_loc, c.x,c.y,c.z,c.w);
  
  glVertexAttribPointer(shader.pos_attr, polygon->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  
  glUniformMatrix4fv(shader.vertex_transform_loc, 1, false, &ctx->matrix.m00);


  int mode = GL_TRIANGLE_STRIP;
  if(ctx->mode == BLIT3D_POINTS)
    mode = GL_POINTS;
  if(ctx->mode == BLIT3D_TRIANGLES)
    mode = GL_TRIANGLES;
  
  glDrawArrays(mode,0, polygon->length / (polygon->dimensions * 4));
  
  //int err =  glGetError();
  //if(err != 0)
  //  printf("eRR2: %i\n", err);
}

void blit3d_polygon_blit2(blit3d_context * ctx, vertex_buffer ** polygons, u32 count){
  
  if(count == 0)
    return;

  //glUniform1i(ctx->shader.textured_loc, 1);
  for(size_t i = 0; i < ctx->texture_count; i++){
	var tex = ctx->current_texture[i];
	
	if(tex == NULL && i == 0)
	  tex = get_default_tex();
	
	if(tex != NULL){
	  glActiveTexture(GL_TEXTURE0 + i);
	  glBindTexture(GL_TEXTURE_2D, tex->handle->tex);
	}
  }
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
    glEnableVertexAttribArray(j);
    glBindBuffer(GL_ARRAY_BUFFER, polygons[j]->buffer);
    glVertexAttribPointer(j, polygons[j]->dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    j += 1;    
  }

  var shader = ctx->shader;
  
  if(ctx->mode == BLIT3D_TRIANGLES_COLOR)
    shader = ctx->shader2;
  if(ctx->mode == BLIT3D_TRIANGLE_STRIP_COLOR)
    shader = ctx->shader2;
  if(ctx->mode == BLIT3D_TRIANGLE_STRIP_TEXTURE_DEPTH)
    shader = ctx->shader3;

  var c = ctx->color;
  glUniform4f(shader.color_loc, c.x,c.y,c.z,c.w);
  glUniformMatrix4fv(shader.vertex_transform_loc, 1, false, &ctx->matrix.m00);
  if(ctx->mode != BLIT3D_TRIANGLES_COLOR && ctx->mode != BLIT3D_TRIANGLE_STRIP_COLOR)
    glUniformMatrix3fv(shader.uv_transform_loc, 1, false, &ctx->uv_matrix.m00);
  
  int mode = GL_TRIANGLE_STRIP;
  if(ctx->mode == BLIT3D_POINTS)
    mode = GL_POINTS;
  if(ctx->mode == BLIT3D_TRIANGLES)
    mode = GL_TRIANGLES;
  if(ctx->mode == BLIT3D_TRIANGLES_COLOR){
    //glPointSize(3.0);
    mode = GL_TRIANGLES;//GL_LINE_LOOP;
  }
  
  
  if(elements_index != -1){
    glDrawElements(mode, polygons[elements_index]->length / (polygons[elements_index]->dimensions * 4), GL_UNSIGNED_BYTE, 0);

    // draw elements here
  }else{
    glDrawArrays(mode,0, polygons[0]->length / (polygons[0]->dimensions * 4));
  }
  for(u32 i = 0; i < count; i++){
    if((int)i == elements_index)
      continue;
    glDisableVertexAttribArray(j);
  }
  
}

void blit3d_color(blit3d_context * ctx, vec4 color){
  ctx->color = color;
}

void blit3d_bind_textures(blit3d_context * ctx, texture ** tex, size_t cnt){
  for(size_t i = 0; i < cnt; i++){
	ctx->current_texture[i] = tex[i];
  }
  ctx->texture_count = cnt;
}
void blit3d_bind_texture(blit3d_context * ctx, texture * tex){
  blit3d_bind_textures(ctx, &tex, 1);
}




void blit3d_blit_quad(blit3d_context * ctx){
  if(ctx->quad_polygon == NULL){
    f32 d[] = {0 , 0, 1, 0, 0, 1, 1, 1 };
    ctx->quad_polygon = blit3d_polygon_new();
    blit3d_polygon_load_data(ctx->quad_polygon, d, sizeof(d));
    blit3d_polygon_configure(ctx->quad_polygon, 2);
  }
  
  vertex_buffer * vb[2];
  vb[0] = ctx->quad_polygon;
  vb[1] = ctx->quad_polygon;
  blit3d_polygon_blit2(ctx, vb, 2);
}

void blit3d_uv_matrix(blit3d_context * ctx, mat3 uv){
  ctx->uv_matrix = uv;
}
