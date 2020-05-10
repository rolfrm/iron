#version 100
precision mediump float;
varying vec2 v_tex_coord;
uniform sampler2D _texture;
uniform int textured;
uniform vec4 color;

void main() 
{
  if(textured != 0)
     gl_FragColor = texture2D( _texture, v_tex_coord ) * color;   
   else
     gl_FragColor = color;
} 
