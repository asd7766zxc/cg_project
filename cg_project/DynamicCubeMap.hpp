#pragma once
#include <glad/glad.h>
#include "ShaderProgram.hpp"
#include "Camera.hpp"
#include <functional>
class DynamicCubeMap {
public:
	int size = 1024;
	int light_count = 0;
	GLuint environMap;
	GLuint environFBO;
	shared_ptr<ShaderProgram> env_program;
	shared_ptr<ShaderProgram> shadow_program;
	shared_ptr<Camera> light_camera;
	shared_ptr<Camera> env_camera;
	GLuint* depthmap;
	GLuint* depthFBO;
	vector<vec3> axis = {
		{ 1,0,0},
		{-1,0,0},
		{0, 1,0},
		{0,-1,0},
		{0,0, 1},
		{0,0,-1},
	};
	vector<vec3> axis_up = {
		{0,-1,0},
		{0,-1,0},
		{0,0, 1},
		{0,0,-1},
		{0,-1,0},
		{0,-1,0},
	};
	vec3 refract_model_position = { 10,5,5 };
	const unsigned int shadow_width = 1024, shadow_height = 1024;
	DynamicCubeMap(int _size, int light_count, shared_ptr<Camera> light_camera);
	void drawBuffer(std::function<void(shared_ptr<ShaderProgram>)> draw_scence);
};