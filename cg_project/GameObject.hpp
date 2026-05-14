#pragma once
#include "Vec.hpp"
#include "aabb.hpp"
#include "Model.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"

struct voxelspace {
	int dim_x = 0, dim_y = 0, dim_z = 0;
	float voxel_size = 0;
	vec3 box = 0.0;
	vec3 box_corner = 0.0;

	// world voxel space id 
	int corner_x = 0;
	int corner_y = 0;
	int corner_z = 0;

	int voxel_size_p = 5;
	int voxel_size_q = 100;
};
class GameObject {
public:
	const int voxel_size_p = 5;
	const int voxel_size_q = 100;
	float voxel_size = float(voxel_size_p) / voxel_size_q;
	vec3 position;
	vec3 scale;
	vec3 rotation;

	shared_ptr<aabb> bounding_box;

	// physics properties
	vec3 velocity;
	vec3 forces;
	vec3 torque;
	float mass;



	shared_ptr<Model> model;
	shared_ptr<Texture> texture;

	voxelspace voxel_info;
	GLuint voxelTexture;
	GLuint collisionVisualizeTexture;

	GameObject(shared_ptr<Model> mesh, shared_ptr<Texture> texture, vec3 position = vec3(0, 0, 0), vec3 scale = vec3(1, 1, 1), vec3 rotation = vec3(0, 0, 0))
		: model(mesh), texture(texture), position(position), scale(scale), rotation(rotation) {

		bounding_box = make_shared<aabb>();
		bounding_box->ref_obj = this;
		update_aabb();

		glGenTextures(1, &voxelTexture);
		glBindTexture(GL_TEXTURE_3D, voxelTexture);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 256, 256, 256 / 32); // roundup z dimension
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


		glGenTextures(1, &collisionVisualizeTexture);
		glBindTexture(GL_TEXTURE_3D, collisionVisualizeTexture);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 256, 256, 256 / 32); // roundup z dimension
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Local (Mesh coordinate) (mesh -> world/object space)
	mat4 localToWorld() const {
		return
			mat4::trans(position) * // 位移
			mat4::Rz(rotation.z) *
			mat4::Ry(rotation.y) *
			mat4::Rx(rotation.x) * // 把軸轉成對齊
			mat4::scale(scale);
	}
	void update_aabb() {
		bounding_box->reset();
		//using obb(mesh's aabb) to calculate the object's aabb
		for (int s = 0; s < (1<<3); ++s) {
			float x = model->bounding_box.x.get((s >> 0) & 1);
			float y = model->bounding_box.y.get((s >> 1) & 1);
			float z = model->bounding_box.z.get((s >> 2) & 1);

			vec3 v = (localToWorld() * vec4(x, y, z, 1)).toVec3();
			bounding_box->x.adjust(v.x);
			bounding_box->y.adjust(v.y);
			bounding_box->z.adjust(v.z);
		}
	}
	
	void draw(shared_ptr<ShaderProgram> shader_program) {
		shader_program->use();
		glActiveTexture(GL_TEXTURE0);
		texture->bind();
		shader_program->setMat4("model", localToWorld());
		model->draw();
	}

	void voxelize(shared_ptr<ShaderProgram> voxelizer);
};
