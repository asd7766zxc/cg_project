#pragma once
#include "Voxelizer.hpp"
#include <filesystem>
#include <fstream>

const BYTE empty_voxel_grid[512 * 512 * 512] = { 0 };
unsigned int tmp_grid[256 * 256 * 256];
void Voxelizer::voxelize(shared_ptr<GameObject> A, bool onlyscaling) {
	A->update_aabb(onlyscaling);
	auto& bounding_box = A->bounding_box;
	auto& voxel_info = A->voxel_info;
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
	voxel_info.box_corner = vec3(l, b, n);
	voxel_info.box = vec3(x_dimen, y_dimen, z_depth) * voxel_size;

	voxelizer->use();
	voxelizer->setMat4("proj", mat4::ortho(l, vr, b, vt, n, vf));
	voxelizer->setMat4("model", onlyscaling ? mat4::scale(A->scale) : A->localToWorld() * mat4::scale(1.1));
	voxelizer->setInt("zdepth", z_depth);

	glBindTexture(GL_TEXTURE_3D, A->voxelTexture);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, x_dimen, y_dimen, (z_depth + 31) / 32, GL_RED_INTEGER, GL_UNSIGNED_BYTE, empty_voxel_grid);

	//bind to image unit 0
	glBindImageTexture(0, A->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glViewport(0, 0, x_dimen, y_dimen);

	//glBindFramebuffer(GL_FRAMEBUFFER, voxelize_fbo);
	A->model->draw();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// the setting of collision visualize texture is the same as voxel texture
	glBindTexture(GL_TEXTURE_3D, A->collisionVisualizeTexture);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, x_dimen, y_dimen, (z_depth + 31) / 32, GL_RED_INTEGER, GL_UNSIGNED_BYTE, empty_voxel_grid);
}
BYTE distance_field_tmp[256 * 256 * 256 * 4]; //4 byte for a float32
bool Voxelizer::read_distace_field_cache(shared_ptr<GameObject> A) {
	std::string cache_path = "cache/" + std::to_string(A->model->VAO) + "_df.bin";
	std::ifstream cache_file(cache_path,std::ios::in | std::ios::binary);
	if (!cache_file.is_open()) {
		return false;
	}
	glBindTexture(GL_TEXTURE_3D, A->distanceTexture);
	int size = sizeof(distance_field_tmp);
	cache_file.read(reinterpret_cast<char*>(distance_field_tmp), size);

	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, df_voxel_dim, df_voxel_dim, df_voxel_dim, GL_RED, GL_FLOAT, distance_field_tmp);
	return true;
}
void write_distance_field_cache(shared_ptr<GameObject> A) {
	std::filesystem::create_directories("cache/");
	std::string cache_path = "cache/" + std::to_string(A->model->VAO) + "_df.bin";
	std::ofstream cache_file(cache_path, std::ios::out | std::ios::binary);
	if (!cache_file.is_open()) {
		return;
	}
	glBindTexture(GL_TEXTURE_3D, A->distanceTexture);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, distance_field_tmp);
	cache_file.write(reinterpret_cast<char*>(distance_field_tmp), sizeof(distance_field_tmp));
}
void Voxelizer::calculate_distance_field(shared_ptr<GameObject> A) {
	if (read_distace_field_cache(A)) return;

	distance_field_compute->use();
	distance_field_compute->setInt("df_resolution", df_voxel_dim);
	distance_field_compute->setInt("vertex_count", A->model->vertex_count);
	A->model->bind_buffer();
	glBindTexture(GL_TEXTURE_3D, A->distanceTexture);
	glBindImageTexture(0, A->distanceTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
	glDispatchCompute((df_voxel_dim + 7) / 8, (df_voxel_dim + 7) / 8, (df_voxel_dim + 7) / 8);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	write_distance_field_cache(A);
}

struct ivec3 {
	int x = 0;
	int y = 0;
	int z = 0;

	int voxel_count = 0;
} zero;
// can be only invoke once
void Voxelizer::calculate_gravitycenter(shared_ptr<GameObject> A) {
	voxelize(A, true);
	bind_buffers();
	glBindImageTexture(0, A->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, center_ssbo);

	auto& voxel_info = A->voxel_info;
	gravitycenter_compute->use();
	gravitycenter_compute->setInt("voxelInfoA.dimx", voxel_info.dim_x);
	gravitycenter_compute->setInt("voxelInfoA.dimy", voxel_info.dim_y);
	gravitycenter_compute->setInt("voxelInfoA.dimz", voxel_info.dim_z);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ivec3), &zero, GL_DYNAMIC_READ);

	glDispatchCompute((voxel_info.dim_x + 7) / 8, (voxel_info.dim_y + 7) / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	ivec3* raw_point = (ivec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	vec3 raw_center = (1.0f / raw_point->voxel_count) * vec3(raw_point->x, raw_point->y, raw_point->z);
	raw_center += vec3(0.5); // the voxel centers' center
	raw_center *= voxel_info.voxel_size; // scaling from voxelspace to world space
	//raw_center += voxel_info.box_corner; // this is to world
	A->gravity_center = raw_center + voxel_info.box_corner;
	A->voxel_info.voxel_count = raw_point->voxel_count;
}

void Voxelizer::calculate_buoyancycenter(shared_ptr<GameObject> A) {
	voxelize(A);
	bind_buffers();
	glBindImageTexture(0, A->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buoyance_ssbo);

	auto& voxel_info = A->voxel_info;
	buoyancycenter_compute->use();
	buoyancycenter_compute->setInt("voxelInfoA.dimx", voxel_info.dim_x);
	buoyancycenter_compute->setInt("voxelInfoA.dimy", voxel_info.dim_y);
	buoyancycenter_compute->setInt("voxelInfoA.dimz", voxel_info.dim_z);


	//buoyancycenter_compute->setVec4("plane")

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ivec3), &zero, GL_DYNAMIC_READ);

	glDispatchCompute((voxel_info.dim_x + 7) / 8, (voxel_info.dim_y + 7) / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	ivec3* raw_point = (ivec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	vec3 raw_center = (1.0f / raw_point->voxel_count) * vec3(raw_point->x, raw_point->y, raw_point->z);
	raw_center += vec3(0.5); // the voxel centers' center
	raw_center *= voxel_info.voxel_size; // scaling from voxelspace to world space
	//raw_center += voxel_info.box_corner; // this is to world
	A->buoyancy_center = raw_center + voxel_info.box_corner;
}


//this is an unit inertia
void Voxelizer::calculate_tensorOfInertia(shared_ptr<GameObject> A) {
	auto& info = A->voxel_info;
	glBindTexture(GL_TEXTURE_3D, A->voxelTexture);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER,GL_UNSIGNED_INT,tmp_grid);
	vec3 sum;
	float voxel_count = A->voxel_info.voxel_count;
	A->inertia.makeZero();
	float ixx = 0;
	float iyy = 0;
	float izz = 0;

	float ixz = 0;
	float ixy = 0;
	float iyz = 0;

	float pmass = (1.0 / voxel_count); // particle mass
	for (int x = 0; x < info.dim_x; ++x) {
		for (int y = 0; y < info.dim_y; ++y) {
			for (int zc = 0; zc < (31 + info.dim_z) / 32; ++zc) {
				int ind = x + y * 256 + zc * 256 * 256; // column major
				for (int j = 0; j < 32; ++j) {
					if (zc * 32 + j >= info.dim_z) break;
					if ((tmp_grid[ind] >> j) & 1) {
						++voxel_count;
						vec3 p = vec3(x, y, zc * 32 + j) + vec3(0.5);
						p *= voxel_size;
						p += A->voxel_info.box_corner;
						p = p - A->gravity_center;
							
						ixx += (p.y * p.y + p.z * p.z) * pmass; // (dis to x)^2
						iyy += (p.x * p.x + p.z * p.z) * pmass;
						izz += (p.x * p.x + p.y * p.y) * pmass;

						ixy += (p.x * p.y) * pmass;
						ixz += (p.x * p.z) * pmass;
						iyz += (p.y * p.z) * pmass;
					}
				}
			}
		}
	}

	auto& mt = A->inertia;

	mt[0] = ixx, mt[1] = -ixy, mt[2] = -ixz; //mt[3]
	mt[4] = -ixy, mt[5] = iyy, mt[6] = -iyz; //mt[7]
	mt[8] = -ixz, mt[9] = -iyz, mt[10] = izz; //mt[11]
	mt[12] = 0, mt[13] = 0, mt[14] = 0; mt[15] = 1.0f;

	A->inverse_inertia = A->inertia.inverse();
}