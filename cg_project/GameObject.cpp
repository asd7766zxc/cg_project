#include "GameObject.hpp"
const BYTE empty_voxel_grid[512 * 512 * 512] = { 0 };
void GameObject::voxelize(shared_ptr<ShaderProgram> voxelizer) {
	//glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	// perform orthognal projection
	// bounding_box -> [-1,1]^2 x [0,z]
	
	//the whole voxel space need to be aligned with world's regular grid;

	//so l, b, n need to be k * voxel_size, (where k is some integer).
	// l  == p / q * k
	// l * q = p * k
	// k == l * q / p
	//TODO: i will fix the roundoff in this equation after major functions are done, now just make sure the voxelizer works
	auto align_with_world = [&](float v) {
		int k = int(v * voxel_size_q / voxel_size_p);
		return float(k) * voxel_size_p / voxel_size_q;
	};
	float l = bounding_box->x.min;
	voxel_info.corner_x = int(l * voxel_size_q / voxel_size_p);
	l = align_with_world(l);
	
	float r = bounding_box->x.max;

	float b = bounding_box->y.min;
	voxel_info.corner_y = int(b * voxel_size_q / voxel_size_p);
	b = align_with_world(b);
	
	float t = bounding_box->y.max;

	float n = bounding_box->z.min;
	voxel_info.corner_z = int(n * voxel_size_q / voxel_size_p);
	n = align_with_world(n);

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
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, x_dimen, y_dimen, (z_depth + 31) / 32, GL_RED_INTEGER, GL_UNSIGNED_BYTE, empty_voxel_grid);

	//bind to image unit 0
	glBindImageTexture(0, voxelTexture,0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glViewport(0, 0, x_dimen, y_dimen);
	model->draw();


	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// the setting of collision visualize texture is the same as voxel texture
	glBindTexture(GL_TEXTURE_3D, collisionVisualizeTexture);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, x_dimen, y_dimen, (z_depth + 31) / 32, GL_RED_INTEGER, GL_UNSIGNED_BYTE, empty_voxel_grid);
}
