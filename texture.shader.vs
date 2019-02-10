#version 300 es

uniform mat3 vertex_transform;
uniform mat3 uv_transform;
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex_coord;
out vec2 v_tex_coord;

void main() 
{
    gl_Position = vec4(vertex_transform * vec3(pos, 1), 1);
    vec3 t = uv_transform * vec3(tex_coord, 1);
    v_tex_coord = t.xy / t.z;    
}