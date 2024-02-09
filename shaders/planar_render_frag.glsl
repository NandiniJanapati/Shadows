#version 120


#define NUM_LIGHTS 1

struct lightStruct
{
	vec3 position;
	vec3 color;
};

uniform lightStruct lights[NUM_LIGHTS];

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;


///---------nandini
varying vec3 N;
varying vec3 Pos;

void main()
{
	//gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);


	///---------nandini
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
