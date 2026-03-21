#version 430 core

in vec4 fragPos;
in vec2 TexCoord;

struct PointLight{
	vec3 position;
	vec3 attenuation; // ax^2 + bx + c

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	int enable;
};

struct Material{
	vec3 emission;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

#define POINT_LIGHTS_COUNT 8
uniform PointLight point_lights[POINT_LIGHTS_COUNT];
uniform Material material;
uniform vec3 view_position;
uniform int isLight;

uniform int current_light;
uniform float far_plane;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int engraved;

void main(){
	float light_distance = length(fragPos.xyz - point_lights[current_light].position);
	light_distance = light_distance / far_plane;
	if(texture(texture2, TexCoord).g <= 0.5f && engraved == 1){
		discard;
	}
	gl_FragDepth = light_distance;
}