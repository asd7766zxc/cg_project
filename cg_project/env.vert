#version 430 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out VS_OUT{
	out vec3 pixelPos;
	out vec3 pixelNorm;
	out vec2 TexCoord;
} vs_out;
uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 textureMat;

void main(){
	gl_Position = model * vec4(vPos, 1.0);
	vs_out.pixelPos = vec3(model * vec4(vPos,1.0));
	vs_out.pixelNorm = vec3(transpose(inverse(model)) * vec4(vNormal, 1.0));
	vec4 tmpTexCoords = textureMat * vec4(vTexCoords.x,vTexCoords.y,0.0,1.0);
	vs_out.TexCoord = tmpTexCoords.xy;
}