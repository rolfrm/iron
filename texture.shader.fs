#version 300 es
precision mediump float;
in vec2 v_tex_coord;
out vec4 out_color;
uniform sampler2D _texture;
uniform int textured;
uniform vec4 color;

void main() 
{
  if(textured != 0)
     out_color = texture( _texture, v_tex_coord );   
  else
    out_color = color;
} 
