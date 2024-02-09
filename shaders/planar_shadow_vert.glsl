#version 120

attribute vec3 vPositionModel; // in object space
attribute vec3 vNormalModel; // in object space

//--nandini

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


uniform vec3 gndNormal;
uniform vec3 gndPoint;
uniform vec3 lightpos; //ray origin
//--nandini

varying vec3 color;

void main()
{	
	gl_Position = vec4(0.0f, 0.0f, 0.0f, 0.0f); 

	//gl_Position = projection * view * model * vec4(vPositionModel, 1.0); //nandini

	vec4 position_H = model * vec4(vPositionModel, 1.0); //position in homogenous coords multiplied by model matrix to get world space
	vec3 tmp = {position_H.x/position_H.w, position_H.y/position_H.w, position_H.z/position_H.w}; //divide by w to get back to homogenous coords
	//project the position onto plane
	vec3 raydir = normalize(position_H.xyz - lightpos);
	float t = dot((gndPoint - lightpos), gndNormal) / (dot(raydir, gndNormal));

	if(t >= 0){
		vec3 shadowpos = lightpos + ((t-0.001) * raydir); //the final position in world space
		gl_Position = projection * view * vec4(shadowpos, 1.0); //transformed to ndc
	}
	else{
		//gl_Position = vec4(0.0f, 0.0f, 0.0f, 0.0f); //i dont know what i should do with vertices that don't intersect with the ground plane
	}

	color = vec3(1.0f, 0.0f, 0.0f);


}
