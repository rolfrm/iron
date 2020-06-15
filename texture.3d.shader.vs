#version 100
precision highp float;
uniform mat4 vertex_transform;
uniform mat3 uv_transform;
attribute vec3 pos;
attribute vec2 tex_coord;
varying vec2 v_tex_coord;

void main() 
{
    gl_Position = vertex_transform * vec4(pos, 1);
  //  vec3 t = uv_transform * vec3(tex_coord, 1);
    //v_tex_coord = t.xy / t.z;
    v_tex_coord =tex_coord;
}