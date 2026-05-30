#pragma once
#include "Vec.hpp"
#include "aabb.hpp"
#include <glad/glad.h>

class Model {
public:
	GLuint VAO, VBO, EBO;
	GLuint triangles, normals;
	aabb bounding_box;
	float minimum_vertex_distance = 0.0f;
	float maximum_vertex_distance = 0.0f;
	float box_size = 0.0f;
	int vertex_count = -1;
	Model();
	Model(float* vertices, int size, int vertex_count = -1);
	void initializeBuffers();
	mat4 meshToField() const;
	mat4 fieldToMesh() const;
	void bind_buffer();
	void draw();
};