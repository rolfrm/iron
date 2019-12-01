#version 100

uniform mat3 vertex_transform;
uniform mat3 uv_transform;
attribute vec2 pos;
attribute vec2 tex_coord;
varying out vec2 v_tex_coord;

void main() 
{
    gl_Position = vec4(vertex_transform * vec3(pos, 1), 1);
    vec3 t = uv_transform * vec3(tex_coord, 1);
    v_tex_coord = t.xy / t.z;    
}