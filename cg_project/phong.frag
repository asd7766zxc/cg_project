#version 430 core

in vec3 pixelPos;
in vec3 pixelNorm;
in vec2 TexCoord;

out vec4 color;

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
#define POINT_LIGHTS_COUNT 4
uniform PointLight point_lights[POINT_LIGHTS_COUNT];
uniform Material material;
uniform vec3 view_position;
uniform int isLight;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform samplerCube depthmap[POINT_LIGHTS_COUNT];

uniform float far_plane;
uniform int showing_depth_map;
uniform int engraved;
uniform vec3 solid_color;

float CalculateShadow(int i){
	vec3 lightToPixel = pixelPos - point_lights[i].position;
	float bias = 0.05;
	float shadow = 0.0;
	float currentDepth = length(lightToPixel);
	for(int x = -1; x <= 1; ++x){
		for(int y = -1; y <= 1; ++y){
			for(int z = -1; z <= 1; ++z){
				float closestDepth = texture(depthmap[i], normalize(lightToPixel) + 0.002 * normalize(vec3(x,y,z))).x;
				closestDepth *= far_plane;
				color += vec4(vec3(closestDepth) / POINT_LIGHTS_COUNT, 1.0) / 27.0;
				shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
			}
		}
	}
	return shadow / 27.0;
}

void main(){
	if(isLight != 0){
		if(isLight < 0){
			color = vec4(1);
			return;
		}
		color = vec4(point_lights[isLight-1].diffuse,1.0f);
		return;
	}
	vec3 N = normalize(pixelNorm);
	vec3 V = normalize(view_position - pixelPos);
	vec3 overall_light_color;
	vec3 light_color;
	for(int i = 0; i < POINT_LIGHTS_COUNT; ++i){
		if(point_lights[i].enable == 0) continue;
		vec3 L = normalize(point_lights[i].position - pixelPos);
		vec3 H = (L + V) / 2.0; //Half-way vector, blinn phong
		vec3 R = reflect(-L, N);

		vec3 ambient = material.ambient * point_lights[i].ambient;
		vec3 diffuse = material.diffuse * max(dot(L,N),0.0) * point_lights[i].diffuse;
		vec3 specular = material.specular * pow(max(dot(R,V),0.0), material.shininess) * point_lights[i].specular;

		//Attenuation 
		float a = point_lights[i].attenuation.x;
		float b = point_lights[i].attenuation.y;
		float c = point_lights[i].attenuation.z;
		float x = length(point_lights[i].position - pixelPos);
		float attenuation = 1.0 / (a * x * x + b * x + c);
	
		float shadow = CalculateShadow(i);
		attenuation = (1.0 - shadow) * attenuation;
		light_color += material.ambient * ambient + material.emission;
		overall_light_color += attenuation * (diffuse + specular);
	}
	if(showing_depth_map == 0){
		light_color += overall_light_color;
		float mx = max(max(light_color.x,light_color.y),light_color.z);
		if(mx >= 1) light_color /= mx;
		color = texture(texture1, TexCoord) * vec4(light_color,1.0) * vec4(solid_color,1.0); 	
	}
	if(texture(texture2, TexCoord).g <= 0.5f && engraved == 1){
		discard;
	}
}