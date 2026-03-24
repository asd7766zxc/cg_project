#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 spaceMatrices[6];

in VS_OUT{
	in vec3 pixelPos;
	in vec3 pixelNorm;
	in vec2 TexCoord;
} gs_in[];
out vec4 fragPos;
out vec2 TexCoord;
out vec3 pixelPos;
out vec3 pixelNorm;

void main(){
	for(int face = 0; face < 6; ++face)	{
		gl_Layer = face;
		for(int i = 0; i < 3; ++i){
			fragPos = gl_in[i].gl_Position; // use to calculate depth
			TexCoord = gs_in[i].TexCoord;
			pixelPos = gs_in[i].pixelPos;
			pixelNorm = gs_in[i].pixelNorm;
			gl_Position = spaceMatrices[face] * fragPos; // specify the projected location
			EmitVertex();
		}
		EndPrimitive();
	}
}
