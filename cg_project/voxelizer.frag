#version 430 core

layout(binding = 0, r32ui) uniform uimage3D voxelGrid;

in vec3 pixelPos;

uniform int zdepth;
out vec4 color;

void main(){
	ivec2 window_xy = ivec2(gl_FragCoord.xy); 
	//int z_index = int(gl_FragCoord.z * zdim + zdim); // [-1,1] -> [0,zdepth - 1]
	int z_index = int(pixelPos.z * (zdepth));
	int z_chunk = z_index / 32;
	int z_subindex = z_index % 32;
	// flip all bits behind z_subindex
	//32 - z_subindex
	//leftmost align with z_subindex
	// 31 -> z_subindex
	// 31 - (31 - zubindex) 
	imageAtomicXor(voxelGrid, ivec3(window_xy.xy,z_chunk), (0xFFFFFFFFu >> (31 - z_subindex)));
	//imageAtomicOr(voxelGrid, ivec3(window_xy,z_chunk),(1u << z_subindex));
	// solid binary voxelization 
	// xor toward -z
	for(int zi = z_chunk - 1; zi >= 0; --zi){
		imageAtomicXor(voxelGrid, ivec3(window_xy.xy,zi),0xFFFFFFFFu); //flip all bit behind (<= current z) 
	}
	//not draw
	discard;

}