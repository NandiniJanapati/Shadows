#version 120

attribute vec3 vPositionModel; // in object space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vec4(vPositionModel, 1.0);	

}
