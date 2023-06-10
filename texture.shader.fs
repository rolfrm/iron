// texture.shader.fs
#version 330
//precision mediump float;
in vec2 v_tex_coord;
uniform sampler2D _texture;
uniform vec4 color;

in vec3 v_vertex_color;
out vec4 FragColor;
void main() 
{
	 FragColor = texture( _texture, v_tex_coord ) * color * vec4(v_vertex_color, 1);
} 
