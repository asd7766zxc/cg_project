#include "GameObject.hpp"

void GameObject::voxelize(float voxel_size, shared_ptr<ShaderProgram> voxelizer) {
	//glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	// perform orthognal projection
	// bounding_box -> [-1,1]^2 x [0,z]
	float l = bounding_box->x.min;
	float r = bounding_box->x.max;

	float b = bounding_box->y.min;
	float t = bounding_box->y.max;

	float n = bounding_box->z.min;
	float f = bounding_box->z.max;

	//calculate voxel space dimension
	int z_depth = int((f - n) / voxel_size) + 1;
	int x_dimen = int((r - l) / voxel_size) + 1;
	int y_dimen = int((t - b) / voxel_size) + 1;

	float vr = x_dimen * voxel_size + l; // align with the voxel space boundary
	float vt = y_dimen * voxel_size + b;
	float vf = z_depth * voxel_size + n;
	
	voxel_info.voxel_size = voxel_size;
	voxel_info.dim_x = x_dimen;
	voxel_info.dim_y = y_dimen;
	voxel_info.dim_z = z_depth;
	voxel_info.box_corner = vec3(l,b,n);
	voxel_info.box = vec3(x_dimen,y_dimen,z_depth) * voxel_size;
	voxelizer->use();
	voxelizer->setMat4("proj",mat4::ortho(l,vr,b,vt,n,vf));
	voxelizer->setMat4("model", localToWorld());
	voxelizer->setInt("zdepth", z_depth);

	glBindTexture(GL_TEXTURE_3D, voxelTexture);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, x_dimen, y_dimen, z_depth, GL_RED_INTEGER, GL_UNSIGNED_BYTE, empty_voxel_grid);

	//bind to image unit 0
	glBindImageTexture(0, voxelTexture,0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glViewport(0, 0, x_dimen, y_dimen);
	model->draw();


	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void GameObject::draw_voxel(shared_ptr<ShaderProgram> visualizer, shared_ptr<Model> cube_mesh) {
	//return;
	glBindTexture(GL_TEXTURE_3D, voxelTexture);
	glBindImageTexture(0, voxelTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
	visualizer->use();
	glBindVertexArray(cube_mesh->VAO);
	int voxel_count = voxel_info.dim_x * voxel_info.dim_y * voxel_info.dim_z;
	visualizer->setInt("xdim", voxel_info.dim_x);
	visualizer->setInt("ydim", voxel_info.dim_y);
	visualizer->setInt("zdim", voxel_info.dim_z);
	visualizer->setVec3("box_dimension",voxel_info.box);
	visualizer->setFloat("voxel_size",voxel_info.voxel_size);
	visualizer->setVec3("box_corner_pos", voxel_info.box_corner);


	glDrawArraysInstanced(GL_TRIANGLES, 0, cube_mesh->vertex_count, voxel_count);
	glBindVertexArray(0);
}