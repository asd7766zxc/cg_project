#version 430 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out vec3 pixelPos;

uniform mat4 model;
uniform mat4 proj;

void main(){
	gl_Position = proj * model * vec4(vPos, 1.0);
	pixelPos = vec3(proj * model * vec4(vPos,1.0));
}