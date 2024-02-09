#version 120

attribute vec3 vPositionModel; // in object space
attribute vec3 vNormalModel; // in object space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


///---------nandini
varying vec3 N;
varying vec3 Pos;


void main()
{
	gl_Position = projection * view * model * vec4(vPositionModel, 1.0);	
	

	///---------nandini
	vec4 normal_H = model * vec4(vNormalModel, 0.0); //normal vector in homogenous coords
	N = normal_H.xyz;
	N = normalize(N);

	vec4 position_H = model * vec4(vPositionModel, 1.0); //position vector in homogenous coords
	vec3 tmp = {position_H.x/position_H.w, position_H.y/position_H.w, position_H.z/position_H.w};
	Pos = tmp;

}
