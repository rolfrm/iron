#version 100
precision mediump float;
varying vec2 v_tex_coord;
varying vec4 out_color;
uniform sampler2D _texture;
uniform int textured;
uniform vec4 color;

void main() 
{
  if(textured != 0)
     gl_FragColor = texture2D( _texture, v_tex_coord );   
  else
    gl_FragColor = color;
} 
