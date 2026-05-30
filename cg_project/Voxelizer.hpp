#pragma once
#include "ShaderProgram.hpp"
#include "GameObject.hpp"
class Voxelizer {
public:
	shared_ptr<ShaderProgram> voxelizer;
	shared_ptr<ShaderProgram> gravitycenter_compute;
	shared_ptr<ShaderProgram> buoyancycenter_compute;
	shared_ptr<ShaderProgram> distance_field_compute;

	GLuint voxelize_fbo;
	GLuint center_ssbo;
	GLuint buoyance_ssbo;

	const int voxel_size_p = 5;
	const int voxel_size_q = 100;
	float voxel_size = float(voxel_size_p) / voxel_size_q;

	const int df_voxel_dim = 256;
	void bind_buffers() {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, center_ssbo); // the center buffer is on binding 0
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buoyance_ssbo); // the buoyance buffer is on binding 1
	}
	Voxelizer() {
		glGenFramebuffers(1, &voxelize_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, voxelize_fbo);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenBuffers(1, &center_ssbo);
		glGenBuffers(1, &buoyance_ssbo);

		voxelizer = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("voxelizer.vert", GL_VERTEX_SHADER),
				make_shared<Shader>("voxelizer.frag", GL_FRAGMENT_SHADER)
		});

		gravitycenter_compute = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("center_calculator.comp", GL_COMPUTE_SHADER)
		});

		buoyancycenter_compute = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("buoyancy_center.comp", GL_COMPUTE_SHADER)
		});

		distance_field_compute = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("distance_field.comp", GL_COMPUTE_SHADER)
		});
	}
	//proxy style
	void voxelize(shared_ptr<GameObject> A, bool onlyscaling = false);
	bool read_distace_field_cache(shared_ptr<GameObject> A);
	void calculate_distance_field(shared_ptr<GameObject> A);
	void calculate_gravitycenter(shared_ptr<GameObject> A);
	void calculate_tensorOfInertia(shared_ptr<GameObject> A);
	void calculate_buoyancycenter(shared_ptr<GameObject> A);
};