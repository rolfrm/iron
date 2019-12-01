#version 100
precision mediump float;
varying in vec2 v_tex_coord;
varying out vec4 out_color;
uniform sampler2D _texture;
uniform int textured;
uniform vec4 color;

void main() 
{
  if(textured != 0)
     out_color = texture2D( _texture, v_tex_coord );   
  else
    out_color = color;
} 
