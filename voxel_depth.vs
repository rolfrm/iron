#version 300 es

precision highp float;

uniform mat4 vertex_transform;

in vec3 pos;
out vec3 model_space;

void main() 
{
	 model_space = pos;
    gl_Position = vertex_transform * vec4(pos, 1.0);
}