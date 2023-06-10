//texture.3d.shader.vs
#version 330
uniform mat4 vertex_transform;
uniform mat3 uv_transform;
in vec3 pos;
in vec2 tex_coord;
out vec2 v_tex_coord;
out vec3 v_vertex_color;

void main() 
{
	 gl_Position = vertex_transform * vec4(pos, 1);
	 vec3 t = uv_transform * vec3(tex_coord, 1);	
	 v_tex_coord = t.xy;
    v_vertex_color = vec3(1);
}

