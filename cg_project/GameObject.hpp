#pragma once
#include "Vec.hpp"
#include "aabb.hpp"
#include "Model.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"

struct voxelspace {
	int dim_x = 0, dim_y = 0, dim_z = 0;
	vec3 box = 0.0;
	vec3 box_corner = 0.0;
	float voxel_size = 0.0f;
	// world voxel space id 
	int corner_x = 0;
	int corner_y = 0;
	int corner_z = 0;

	int voxel_count = 0;
};
class GameObject {
public:
	vec3 scale;

	shared_ptr<aabb> bounding_box;
	shared_ptr<Model> model;
	shared_ptr<Texture> texture;

	voxelspace voxel_info;
	GLuint voxelTexture;
	GLuint collisionVisualizeTexture;

	GameObject(shared_ptr<Model> mesh, shared_ptr<Texture> texture, vec3 position = vec3(0, 0, 0), vec3 scale = vec3(1, 1, 1), vec3 rotation = vec3(0, 0, 0))
		: model(mesh), texture(texture), position(position), scale(scale) {

		bounding_box = make_shared<aabb>();
		bounding_box->ref_obj = this;
		update_aabb();

		glGenTextures(1, &voxelTexture);
		glBindTexture(GL_TEXTURE_3D, voxelTexture);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 256, 256, 256 / 32); // roundup z dimension
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // the 256x256x256 size ensure every size within i32
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


		glGenTextures(1, &collisionVisualizeTexture);
		glBindTexture(GL_TEXTURE_3D, collisionVisualizeTexture);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 256, 256, 256 / 32); // roundup z dimension
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void update_aabb(bool onlyscaling = false) {
		bounding_box->reset();
		//using obb(mesh's aabb) to calculate the object's aabb
		for (int s = 0; s < (1<<3); ++s) {
			float x = model->bounding_box.x.get((s >> 0) & 1);
			float y = model->bounding_box.y.get((s >> 1) & 1);
			float z = model->bounding_box.z.get((s >> 2) & 1);

			vec3 v = ((onlyscaling ? mat4::scale(scale) : localToWorld()) * vec4(x, y, z, 1)).toVec3();
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

	//special parameters
	bool penetrable = false; // ghost 
	float density = 1.0f; //water

	// physics properties
	
	//centers
	vec3 gravity_center; // body coord 
	vec3 buoyancy_center; // world coord

	//eulers
	vec3 position; // gravity_center pos
	
	vec3 impulsed_acc;

	vec3 velocity;
	vec3 rotation;

	//changes
	vec3 accumlatedForces;
	vec3 accumulatedTorque;
	
	//inertia
	float mass = 1.0f;
	mat4 inertia;
	mat4 inverse_inertia;

	quat orientation = quat(0,0,1,0); // the default orientation

	float velocity_damping = 0.9999;
	float angular_damping = 0.9999;

	// if mass < 0 -> infinite mass
	float inverseMass() const {
		if (mass < 0) return -1.0f;
		return 1.0 / mass;
	}
	bool hasInifiniteMass() const {
		return mass < 0;
	}
	vec3 getWorldGravityCenter() const {
		return (toWorld() * vec4(vec3(0.0), 1)).toVec3();
	}
	mat4 inverseInertiaWorld() {
		// RIR^-1 (座標變換而已) position dosen't matter
		// R^-1 (to local)
		// R (to world)
		auto rotmat = mat4::quat(orientation);
		return rotmat * inverse_inertia * rotmat.transposed();
	}
	void integrate(float dt) {
		if (hasInifiniteMass()) return;
		vec3 linear_acc = accumlatedForces * inverseMass() + impulsed_acc; //we have impulsed acc (occur when collision)
		vec3 angular_acc = inverseInertiaWorld() * accumulatedTorque;

		//velocity based change
		velocity += linear_acc * dt;
		rotation += angular_acc * dt;

		//to reduce energy
		velocity *= velocity_damping;
		rotation *= angular_damping;

		position += velocity * dt;
		orientation = orientation.rotate(rotation * dt);
		orientation.normalize();
		clearAccumulators();
	}
	void clearAccumulators() {
		impulsed_acc = vec3(0.0);
		accumulatedTorque = vec3(0.0);
		accumlatedForces = vec3(0.0);
	}
	void addForce(vec3 f) {
		accumlatedForces += f;
	}
	//from world point
	void addForceAtPoint(vec3 f, vec3 p) {
		accumulatedTorque += (p - getWorldGravityCenter()) ^ f; // r x f
		addForce(f);
	}
	// Local (Mesh coordinate) (mesh -> world/object space)
	mat4 localToWorld() const {
		return
			mat4::trans(position) * // displacement
			mat4::quat(orientation) * // use the orientation transformation from quaternion
			mat4::trans(-gravity_center) * mat4::scale(scale); // the scaling usually to scale the object's size
	}
	// motion to world
	mat4 toWorld() const {
		return
			mat4::trans(position) * // displacement
			mat4::quat(orientation); // use the orientation transformation from quaternion
	}
};
// force generator list (list of force generators)
// gravity force 
// 