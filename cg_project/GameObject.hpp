#pragma once
#include "Vec.hpp"
#include "aabb.hpp"
#include "Model.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"

enum GeometryType {
	PLANE,
	SPHERE,
};
struct voxelspace {
	int dim_x, dim_y, dim_z;
	float voxel_size;
	vec3 box;
	vec3 box_corner;
};
class GameObject {
public:
	vec3 position;
	vec3 scale;
	vec3 rotation;
	shared_ptr<aabb> bounding_box;
	vec3 velocity;
	vec3 forces;
	shared_ptr<Model> model;
	shared_ptr<Texture> texture;
	GeometryType type;
	voxelspace voxel_info;
	GLuint voxelTexture;
	
	BYTE empty_voxel_grid[100 * 100 * 100];
	GameObject(vec3 position = vec3(0, 0, 0), vec3 scale = vec3(1, 1, 1), vec3 rotation = vec3(0, 0, 0))
		:position(position), scale(scale), rotation(rotation) {
		//bounding_box.ref_obj = shared_ptr<GameObject>(this);
		bounding_box = make_shared<aabb>();
		glGenTextures(1, &voxelTexture);
		glBindTexture(GL_TEXTURE_3D, voxelTexture);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 100, 100, 100); // roundup z dimension
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
		memset(empty_voxel_grid, 0, sizeof(empty_voxel_grid));

	}

	// Local (Mesh coordinate)
	mat4 localToWorld() const {
		return
			mat4::trans(position) * // 位移
			mat4::Rz(rotation.z) *
			mat4::Ry(rotation.y) *
			mat4::Rx(rotation.x) * // 把軸轉成對齊
			mat4::scale(scale);
	}
	virtual void update_aabb() = 0;
	
	virtual void draw(shared_ptr<ShaderProgram> shader_program) = 0;

	void voxelize(float voxel_size, shared_ptr<ShaderProgram> voxelizer);
	void draw_voxel(shared_ptr<ShaderProgram> visualizer, shared_ptr<Model> cube_mesh);
};

class Sphere : public GameObject {
public:
	float radius;
	vec3 center;
	Sphere(float radius, vec3 center, vec3 rotation = vec3(0, 0, 0))
		: GameObject(center, scale, rotation), radius(radius), center(center) {
		type = SPHERE;
		scale = vec3(radius);
	}
	virtual void update_aabb() override {
		bounding_box = make_shared<aabb>();
		bounding_box->x = interval(-radius + position.x, +radius + position.x);
		bounding_box->y = interval(-radius + position.y, +radius + position.y);
		bounding_box->z = interval(-radius + position.z, +radius + position.z);
	}
	void draw(shared_ptr<ShaderProgram> shader_program) override {
		shader_program->setMat4("model", localToWorld());
		glActiveTexture(GL_TEXTURE0);
		texture->bind();
		model->draw();
	}
};

class Plane : public GameObject {
public:
	vec3 center;
	Plane(vec3 center, vec3 scale = vec3(1), vec3 rotation = vec3(0, 0, 0))
		: GameObject(center, scale, rotation), center(center) {
		type = PLANE;
	}
	virtual void update_aabb() override {
		bounding_box = make_shared<aabb>();
		vec3 o(0, 0, 0);
		vec3 u(0, 0, 1);
		vec3 v(1, 0, 0);
		vec3 w = u + v;
		float delta = 0.1;

		o = (localToWorld() * vec4(o, 1.0)).toVec3();
		u = (localToWorld() * vec4(u, 1.0)).toVec3();
		v = (localToWorld() * vec4(v, 1.0)).toVec3();
		w = (localToWorld() * vec4(w, 1.0)).toVec3();

		for (auto e : vector<vec3>{ o,u,v,w }) {
			bounding_box->x.adjust(e.x - delta);
			bounding_box->y.adjust(e.y - delta);
			bounding_box->z.adjust(e.z - delta);
						
			bounding_box->x.adjust(e.x + delta);
			bounding_box->y.adjust(e.y + delta);
			bounding_box->z.adjust(e.z + delta);
		}
	}
	std::pair<vec3, vec3> transformedPlane() {
		vec3 u(0, 0, 1);
		vec3 v(1, 0, 0);
		u = (localToWorld() * vec4(u, 0.0)).toVec3();
		v = (localToWorld() * vec4(v, 0.0)).toVec3();
		return { u,v };
	}
	void draw(shared_ptr<ShaderProgram> shader_program) override {
		shader_program->setMat4("model", localToWorld());
		glActiveTexture(GL_TEXTURE0);
		texture->bind();
		model->draw();
	}
};