#version 130
precision mediump float;
varying vec2 v_tex_coord;
uniform sampler2D _texture;
uniform sampler2D _depthtexture;
uniform vec4 color;

varying vec3 v_vertex_color;

void main() 
{
	float depth = texture2D( _depthtexture, v_tex_coord ).x;
	//if(depth <= gl_FragDepth)
	//     discard;
	 gl_FragColor =	texture2D( _texture, v_tex_coord ) * color * vec4(v_vertex_color, 1) ;
	 gl_FragDepth = depth;
} 
