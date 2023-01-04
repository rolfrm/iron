#version 100
precision mediump float;
varying vec2 v_tex_coord;
uniform sampler2D _texture;
uniform vec4 color;

varying vec3 v_vertex_color;

void main() 
{
	 gl_FragColor = texture2D( _texture, v_tex_coord ) * color * vec4(v_vertex_color, 1);
} 
