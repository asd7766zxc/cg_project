#pragma once
#include "Vec.hpp"
#include "aabb.hpp"
#include <glad/glad.h>

class Model {
public:
	GLuint VAO, VBO, EBO;
	GLuint triangles, normals;
	aabb bounding_box;
	int vertex_count = -1;
	Model();
	Model(float* vertices, int size, int vertex_count = -1);
	void initializeBuffers();
	void bind_buffer();
	void draw();
};