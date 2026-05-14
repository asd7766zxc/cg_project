#version 430 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

layout(binding = 0, r32ui) uniform readonly uimage3D voxelGrid;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec3 pixelPos;
out vec3 pixelNorm;

void main(){
	//just draw a cube
	gl_Position = proj * view * model * vec4(vPos,1.0);

	pixelPos = (view * model * vec4(vPos,1.0)).xyz;
	pixelNorm = vNormal; //w/ no rotation so just output the vertex normal
}