#version 120


#define NUM_LIGHTS 1

struct lightStruct
{
	vec3 position;
	vec3 color;
};

uniform lightStruct lights[NUM_LIGHTS];

uniform sampler2D shadowMap;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;


//
varying vec4 posInLightCoords;

varying vec3 N;
varying vec3 Pos;


void main()
{
	//gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	vec3 fragPosLightSpace = posInLightCoords.xyz / posInLightCoords.w; //ndc
	
	vec3 coords = (fragPosLightSpace * 0.5) + 0.5; //convert ndc to 0,1 coords
	vec2 uv_coords = {coords.x, coords.y}; //coords to look up in shadow map

	if(uv_coords.x > 0.01 && uv_coords.x < 0.99 && uv_coords.y > 0.01 && uv_coords.y < 0.99){ //uv coords are between 0 and 1
		float depth = texture2D(shadowMap, uv_coords).x;

		if((coords.z - 0.001) > depth){
			gl_FragColor = vec4(ka, 1.0f);
		}
		else{
			vec3 eye = {0.0f, 5.0f, 5.0f};
			vec3 E = eye - Pos;
			E = normalize(E);

			vec3 I = ka;
			for(int i = 0; i < NUM_LIGHTS; i++){
				vec3 L = lights[i].position - Pos;
				L = normalize(L);
				vec3 R = 2 * (dot(L,N))*N - L;
				R = normalize(R);
				I = I + lights[i].color * (kd * max(0, dot(L, N)) + ks * pow(max(0, dot(R,E)), s));
			}
	
			gl_FragColor = vec4(I, 1.0f);
		}

	}
	else{
	//outside of what our light camera was able to view/calculate
		vec3 eye = {0.0f, 5.0f, 5.0f};
		vec3 E = eye - Pos;
		E = normalize(E);

		vec3 I = ka;
		for(int i = 0; i < NUM_LIGHTS; i++){
			vec3 L = lights[i].position - Pos;
			L = normalize(L);
			vec3 R = 2 * (dot(L,N))*N - L;
			R = normalize(R);
			I = I + lights[i].color * (kd * max(0, dot(L, N)) + ks * pow(max(0, dot(R,E)), s));
		}
	
		gl_FragColor = vec4(I, 1.0f);
		

	}

	

}