//texture.3d.color.shader.vs
#version 330
uniform mat4 vertex_transform;
in vec3 pos;
in vec3 color;
out vec2 v_tex_coord;
out vec3 v_vertex_color;

void main() 
{
	 gl_Position = vertex_transform * vec4(pos, 1);
	 v_tex_coord = vec2(0);
    v_vertex_color = color;  
    }
