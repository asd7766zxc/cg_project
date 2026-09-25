#pragma once
#include "Vec.hpp"
#include <glad/glad.h>
#include "Camera.hpp"
#include "ShaderProgram.hpp"
#include "MeshBuilder.hpp"
#include "Model.hpp"

class Visualization {
public:
	shared_ptr<ShaderProgram> voxel_visualizer_program;
	shared_ptr<ShaderProgram> aabb_visualizer_program;
	shared_ptr<ShaderProgram> solid_color_program;
	shared_ptr<ShaderProgram> df_visualizer_program;
	shared_ptr<Camera> camera;

	Visualization(shared_ptr<Camera> _camera) : camera(_camera) {
		voxel_visualizer_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("voxelvisualizer.vert", GL_VERTEX_SHADER),
				make_shared<Shader>("voxelvisualizer.frag", GL_FRAGMENT_SHADER)
		});

		aabb_visualizer_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("aabbvisualizer.vert", GL_VERTEX_SHADER),
				make_shared<Shader>("aabbvisualizer.frag", GL_FRAGMENT_SHADER)
		});

		solid_color_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("solidcolor.vert", GL_VERTEX_SHADER),
				make_shared<Shader>("solidcolor.frag", GL_FRAGMENT_SHADER)
		});

		df_visualizer_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("distancefieldvisualization.vert", GL_VERTEX_SHADER),
				make_shared<Shader>("distancefieldvisualization.frag", GL_FRAGMENT_SHADER)
		});
	}

	void update_program_view(shared_ptr<ShaderProgram> program) {
		program->use();
		program->setMat4("view", camera->view);
		program->setMat4("proj", camera->proj);
	}

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
		// head length is 0.025
		// so result length is 0.125
		m = std::max(0.0f,m - scale * 0.5f * 0.5f);
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

	static void draw_voxel(shared_ptr<GameObject> obj, shared_ptr<ShaderProgram> visualizer, shared_ptr<Camera> camera, shared_ptr<Model> cube_mesh) {
		glDisable(GL_DEPTH_TEST);
		visualizer->use();
		visualizer->setVec4("voxel_color", vec4(0.1f, 0.8f, 0.2f, 0.2f));
		visualizer->setMat4("proj", camera->proj);
		visualizer->setMat4("view", camera->view);
		glBindTexture(GL_TEXTURE_3D, obj->voxelTexture);
		glBindImageTexture(0, obj->voxelTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
		visualizer->use();
		glBindVertexArray(cube_mesh->VAO);
		int voxel_count = obj->voxel_info.dim_x * obj->voxel_info.dim_y * obj->voxel_info.dim_z;

		visualizer->setInt("xdim", obj->voxel_info.dim_x);
		visualizer->setInt("ydim", obj->voxel_info.dim_y);
		visualizer->setInt("zdim", obj->voxel_info.dim_z);
		visualizer->setVec3("box_dimension", obj->voxel_info.box);
		visualizer->setFloat("voxel_size", obj->voxel_info.voxel_size);
		visualizer->setVec3("box_corner_pos", obj->voxel_info.box_corner);


		glDrawArraysInstanced(GL_TRIANGLES, 0, cube_mesh->vertex_count, voxel_count);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}

	static void draw_voxel_collision(shared_ptr<GameObject> obj, shared_ptr<ShaderProgram> visualizer, shared_ptr<Camera> camera, shared_ptr<Model> cube_mesh) {
		glDisable(GL_DEPTH_TEST);
		visualizer->use();
		visualizer->setVec4("voxel_color", vec4(0.8f, 0.1f, 0.2f, 0.8f));
		visualizer->setMat4("proj", camera->proj);
		visualizer->setMat4("view", camera->view);
		glBindTexture(GL_TEXTURE_3D, obj->collisionVisualizeTexture);
		glBindImageTexture(0, obj->collisionVisualizeTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
		visualizer->use();
		glBindVertexArray(cube_mesh->VAO);
		int voxel_count = obj->voxel_info.dim_x * obj->voxel_info.dim_y * obj->voxel_info.dim_z;
		visualizer->setInt("xdim", obj->voxel_info.dim_x);
		visualizer->setInt("ydim", obj->voxel_info.dim_y);
		visualizer->setInt("zdim", obj->voxel_info.dim_z);
		visualizer->setVec3("box_dimension", obj->voxel_info.box);
		visualizer->setFloat("voxel_size", obj->voxel_info.voxel_size);
		visualizer->setVec3("box_corner_pos", obj->voxel_info.box_corner);

		glDrawArraysInstanced(GL_TRIANGLES, 0, cube_mesh->vertex_count, voxel_count);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}
	static void draw_distance(shared_ptr<GameObject> obj, shared_ptr<ShaderProgram> visualizer, shared_ptr<Camera> camera, shared_ptr<Model> cube_mesh,float slice) {
		//glDisable(GL_DEPTH_TEST);
		visualizer->use();
		visualizer->setMat4("proj", camera->proj);
		visualizer->setMat4("view", camera->view);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, obj->distanceTexture);

		visualizer->use();
		glBindVertexArray(cube_mesh->VAO);
		int voxel_count = 1 * 1 * 1;
		visualizer->setFloat("slice",-slice);
		visualizer->setInt("xdim", 1);
		visualizer->setInt("ydim", 1);
		visualizer->setInt("zdim", 1);
		visualizer->setVec3("box_dimension", vec3(4.0f));
		visualizer->setFloat("voxel_size", 4.0f);
		visualizer->setVec3("box_corner_pos", vec3(-6,-6,-6));

		glDrawArraysInstanced(GL_TRIANGLES, 0, cube_mesh->vertex_count, voxel_count);
		glBindVertexArray(0);
		//glEnable(GL_DEPTH_TEST);
	}

	static void draw_aabb(aabb bb, shared_ptr<ShaderProgram> visualizer, shared_ptr<Camera> camera, shared_ptr<Model> cube_mesh) {
		visualizer->use();
		visualizer->setMat4("proj", camera->proj);
		visualizer->setMat4("view", camera->view);
		visualizer->setMat4("model", mat4::trans(vec3(bb.x.min, bb.y.min, bb.z.min)) * mat4::scale(vec3(bb.x.size(), bb.y.size(), bb.z.size())) * mat4::trans(0.5));
		cube_mesh->draw();
	}
	static void draw_glow(shared_ptr<GameObject> obj, shared_ptr<ShaderProgram> program, vec4 color) {

		program->use();

		program->setMat4("model", obj->localToWorld());
		program->setVec4("solid_color", color);
		obj->model->draw();
	}
	shared_ptr<Model> rod_mesh = MeshBuilder::Rod(10);
	shared_ptr<Model> cone_mesh = MeshBuilder::Cone(10);
	shared_ptr<Model> sphere_mesh = MeshBuilder::Sphere(100);
	shared_ptr<Model> cube = MeshBuilder::Cube();

	void draw_vector(vec3 direction, vec3 origin, vec4 color, float scale = 0.2f) {
		update_program_view(solid_color_program);
		Visualization::draw_vector(solid_color_program, rod_mesh, cone_mesh, direction, origin, color, scale);
	};
	void draw_point(vec3 position, vec4 color, float scale = 1.0f) {
		update_program_view(solid_color_program);
		Visualization::draw_point(solid_color_program, sphere_mesh, position, color, scale);
	};
	void draw_voxel(shared_ptr<GameObject> obj) {
		Visualization::draw_voxel(obj, voxel_visualizer_program, camera, cube);
	}
	void draw_voxel_collision(shared_ptr<GameObject> obj) {
		Visualization::draw_voxel_collision(obj, voxel_visualizer_program, camera, cube);
	}
	void draw_distance(shared_ptr<GameObject> obj,float slice = 0.0f) {
		Visualization::draw_distance(obj, df_visualizer_program, camera, cube,slice);
	}
	void draw_aabb(aabb bb) {
		//TODO: draw the axis only
		Visualization::draw_aabb(bb, aabb_visualizer_program, camera, cube);
	}
	void draw_glow(shared_ptr<GameObject> obj, vec4 color) {
		update_program_view(solid_color_program);
		Visualization::draw_glow(obj, solid_color_program,color);
	}

	GLuint line_vao = 0, line_vbo = 0;
	void draw_polyline(const vector<vec3>& pts, vec4 color) {
		if (pts.size() < 2) return;
		if (!line_vao) {
			glGenVertexArrays(1, &line_vao);
			glGenBuffers(1, &line_vbo);
			glBindVertexArray(line_vao);
			glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);
		}
		vector<float> buf;
		buf.reserve(pts.size() * 3);
		for (auto& p : pts) buf.push_back(p.x), buf.push_back(p.y), buf.push_back(p.z);
		glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
		glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.data(), GL_STREAM_DRAW);

		update_program_view(solid_color_program);
		solid_color_program->setMat4("model", mat4::identity());
		solid_color_program->setVec4("solid_color", color);
		glVertexAttrib3f(1, 1.0f, 1.0f, 0.5f);

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(line_vao);
		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)pts.size());
		glPointSize(4.0f);
		glDrawArrays(GL_POINTS, 0, (GLsizei)pts.size());
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}
};