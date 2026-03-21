#pragma once
#include "Vec.hpp"
#include <glad/glad.h>

class Model {
public:
	vec3 position;
	vec3 rotation;
	vec3 scale = vec3(1);
	GLuint VAO, VBO, EBO;
	int vertex_count = -1;
	Model();
	Model(float* vertices, int size, int vertex_count = -1);
	void draw();
};