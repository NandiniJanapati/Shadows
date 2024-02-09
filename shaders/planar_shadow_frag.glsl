#version 120

uniform vec3 ka;

varying vec3 color;

void main()
{
	gl_FragColor = vec4(ka, 1.0f);
}
