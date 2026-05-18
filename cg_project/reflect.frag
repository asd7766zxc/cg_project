#version 430 core

in vec3 pixelPos;
in vec3 pixelNorm;
in vec2 TexCoord;

out vec4 color;
layout(binding = 0) uniform samplerCube envmap;
layout(binding = 1) uniform sampler2D tex;
uniform vec3 camera_position;
uniform int reflection;

void main(){

	vec3 I = normalize(pixelPos - camera_position);
	float ratio = 1.00 / 1.52;;
	vec3 R;
	if(reflection != 0)
		R = reflect(I,normalize(pixelNorm));
	else 
		R = refract(I,normalize(pixelNorm),ratio);
	color = mix(vec4(texture(envmap,R).rgb,1.0),vec4(texture(tex,TexCoord).rgb,1.0),0.5);
}