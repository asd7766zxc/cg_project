#pragma once
#include "Vec.hpp"
#include <glad/glad.h>
#include "Camera.hpp"
#include "ShaderProgram.hpp"
#include "MeshBuilder.hpp"
#include "Model.hpp"

class Visualization {
public:
	static void draw_vector(shared_ptr<ShaderProgram> program\
		,shared_ptr<Model> rod_mesh, shared_ptr<Model> cone_mesh, vec3 direction, vec3 origin, vec4 color,float scale = 1.0f) {
		glDisable(GL_DEPTH_TEST);
		program->use();
		float m = abs(direction);
		direction = uni(direction);

		vec3 up(0, 1, 0);
		vec3 u = direction ^ up;
		if (abs(u) < 1e-8) {
			up = vec3(1, 0, 0);
			u = direction ^ up;
		}
		u = uni(u);
		vec3 v = u ^ direction;
		v = uni(v);
		program->setMat4("model", mat4::trans(origin) * mat4::coord(u,v,direction).transposed() * mat4::scale(vec3(0.2 * scale, 0.2 * scale, m)));
		program->setVec4("solid_color", color);
		rod_mesh->draw();

		program->setMat4("model", mat4::trans(origin + direction * m) * mat4::coord(u, v, direction).transposed() * mat4::scale(scale * vec3(0.2, 0.2, 0.5)));
		program->setVec4("solid_color", color);
		cone_mesh->draw();

		glEnable(GL_DEPTH_TEST);
	}

	static void draw_point(shared_ptr<ShaderProgram> program\
		, shared_ptr<Model> sphere_mesh, vec3 position, vec4 color, float scale = 1.0f) {
		glDisable(GL_DEPTH_TEST);
		program->use();

		program->setMat4("model", mat4::trans(position) * mat4::scale(vec3(0.02 * scale)));
		program->setVec4("solid_color", color);
		sphere_mesh->draw();

		glEnable(GL_DEPTH_TEST);
	}
};