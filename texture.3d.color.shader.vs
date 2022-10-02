#version 100
uniform mat4 vertex_transform;
attribute vec3 pos;
attribute vec3 color;
attribute float point_size;
varying vec2 v_tex_coord;
varying vec3 v_vertex_color;

void main() 
{
	 gl_Position = vertex_transform * vec4(pos, 1);
	 v_tex_coord = vec2(0);
    v_vertex_color = color;  
}
