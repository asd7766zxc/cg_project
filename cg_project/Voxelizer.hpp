#pragma once
#include "ShaderProgram.hpp"
#include "GameObject.hpp"
class Voxelizer {
public:
	shared_ptr<ShaderProgram> voxelizer;
	shared_ptr<ShaderProgram> gravitycenter_compute;
	shared_ptr<ShaderProgram> buoyancycenter_compute;

	GLuint center_ssbo;
	GLuint buoyance_ssbo;

	const int voxel_size_p = 5;
	const int voxel_size_q = 100;
	float voxel_size = float(voxel_size_p) / voxel_size_q;
	Voxelizer() {
		glGenBuffers(1, &center_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, center_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, center_ssbo); // the center buffer is on binding 3

		glGenBuffers(1, &buoyance_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buoyance_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, buoyance_ssbo); // the center buffer is on binding 3

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
	}
	//proxy style
	void voxelize(shared_ptr<GameObject> A, bool onlyscaling = false);
	void calculate_gravitycenter(shared_ptr<GameObject> A);
	void calculate_momentOfInertia(shared_ptr<GameObject> A);
	void calculate_buoyancycenter(shared_ptr<GameObject> A);
};