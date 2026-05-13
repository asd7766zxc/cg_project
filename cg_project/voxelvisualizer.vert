#version 430 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

layout(binding = 0, r32ui) uniform readonly uimage3D voxelGrid;

uniform mat4 proj;
uniform mat4 view;

uniform vec3 box_dimension;
uniform vec3 box_corner_pos;

uniform int xdim;
uniform int ydim;
uniform int zdim;
uniform float voxel_size;


out vec3 pixelPos;
out vec3 pixelNorm;

void main(){
	//draw cube according to voxel grid

	int id = int(gl_InstanceID);
	int block = ydim * zdim;
	int x = id / block;
	int y = (id % block) / zdim;
	int z = (id % block) % zdim;

	int chunkz = z / 32;
	int zbit = z % 32;
	uint data = imageLoad(voxelGrid, ivec3(x,y,chunkz)).r;
	float scale = voxel_size;
	if((data >> zbit & 1) == 0u) scale = 0.0; //not display the voxel

	vec3 pos = vec3(float(x) / xdim,float(y) / ydim,float(z) / zdim) * box_dimension + box_corner_pos;
	// the pos is left bottom corner
	
	pos = pos + (scale * (vPos + vec3(0.5)));
	pixelPos = pos;
	pixelNorm = vNormal; //w/ no rotation so just output the vertex normal

	gl_Position = proj * view * vec4(pos,1.0);
	
}