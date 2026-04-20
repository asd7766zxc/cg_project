#pragma once
#include "Vec.hpp"
#include "aabb.hpp"
#include "Model.hpp"
#include "ShaderProgram.hpp"
class GameObject {
public:
	vec3 position;
	vec3 scale;
	vec3 rotation;
	aabb bounding_box;
	shared_ptr<Model> model;

	GameObject(vec3 position = vec3(0, 0, 0), vec3 scale = vec3(1, 1, 1), vec3 rotation = vec3(0, 0, 0))
		:position(position), scale(scale), rotation(rotation) {
		bounding_box.ref_obj = shared_ptr<GameObject>(this);
	}

	mat4 localToWorld() const {
		return
			mat4::trans(position) * // 位移
			mat4::Rz(rotation.z) *
			mat4::Ry(rotation.y) *
			mat4::Rx(rotation.x) * // 把軸轉成對齊
			mat4::scale(scale);
	}

	virtual bool hit(GameObject& other);
};

class Sphere : public GameObject {
	public:
	float radius;
	vec3 center;
	Sphere(float radius, vec3 center, vec3 scale = vec3(1), vec3 rotation = vec3(0, 0, 0))
		: GameObject(center, scale, rotation), radius(radius), center(center){
		scale = vec3(radius);
	}

	void onHit(vec3 color) {

	}

	void draw(shared_ptr<ShaderProgram> shader_program) {
		shader_program->setMat4("model", localToWorld());

		model->draw();
	}
}