#pragma once
#include <glad/glad.h>
#include "Vec.hpp"
#include "Helpers.hpp"

struct Vertex {
	vec3 position;
	vec3 normal;
	vec3 uv;
	vec3 emission;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
class VertexObject {
public:
	unsigned int VAO, VBO;
	//fomat:
	// position, normal, uv, 
	// material properties (emission, ambient, diffuse, specular, shininess)
	VertexObject(vector<Vertex> vertices);
};