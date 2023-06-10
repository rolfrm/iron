//texture.depth.shader.fs
#version 330
//precision mediump float;
in vec2 v_tex_coord;
uniform sampler2D _texture;
uniform sampler2D _depthtexture;
uniform vec4 color;

in vec3 v_vertex_color;
out vec4 FragColor;
void main() 
{
	float depth = texture( _depthtexture, v_tex_coord ).x;
	//if(depth <= gl_FragDepth)
	//     discard;
	 FragColor =texture( _texture, v_tex_coord ) * color * vec4(v_vertex_color, 1) ;
	 //gl_FragDepth = depth;//(depth + gl_FragCoord.z )* 0.5;
} 
