#pragma once
#include "GameObject.hpp"
#include "Camera.hpp"

class VoxelHelper {
public:
	static void draw_voxel(shared_ptr<GameObject> obj, shared_ptr<ShaderProgram> visualizer, shared_ptr<Camera> camera, shared_ptr<Model> cube_mesh) {
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
	static void draw_aabb(aabb bb, shared_ptr<ShaderProgram> visualizer, shared_ptr<Camera> camera, shared_ptr<Model> cube_mesh) {
		visualizer->use();
		visualizer->setMat4("proj", camera->proj);
		visualizer->setMat4("view", camera->view);
		visualizer->setMat4("model", mat4::trans(vec3(bb.x.min, bb.y.min, bb.z.min)) * mat4::scale(vec3(bb.x.size(), bb.y.size(), bb.z.size())) * mat4::trans(0.5));
		cube_mesh->draw();	
	}

};
