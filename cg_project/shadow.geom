#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 lightSpaceMatrices[6];

in VS_OUT{
	in vec3 pixelPos;
	in vec3 pixelNorm;
	in vec2 gTexCoord;
} gs_in[];
out vec4 fragPos;
out vec2 TexCoord;

void main(){
	for(int face = 0; face < 6; ++face)	{
		gl_Layer = face;
		for(int i = 0; i < 3; ++i){
			fragPos = gl_in[i].gl_Position; // use to calculate depth
			TexCoord = gs_in[i].gTexCoord;
			gl_Position = lightSpaceMatrices[face] * fragPos; // specify the projected location
			EmitVertex();
		}
		EndPrimitive();
	}
}
