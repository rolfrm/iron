#version 100
precision mediump float;
varying vec2 v_tex_coord;
uniform sampler2D _texture;
uniform vec4 color;

void main() 
{
	 gl_FragColor = texture2D( _texture, v_tex_coord ) * color;  
} 
