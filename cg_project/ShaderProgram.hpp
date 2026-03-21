#pragma once
#include <glad/glad.h>
#include "Shader.hpp"
#include "Helpers.hpp"
#include "Vec.hpp"

class ShaderProgram {
public:
	unsigned int ID;
	ShaderProgram(vector<shared_ptr<Shader>> shaders);
	void checkCompileErrors();
	void use();
	void setInt(const std::string& name, int v) const;
	void setFloat(const std::string& name, float v) const;
	void setVec3(const std::string& name, vec3 v) const;
	void setMat4(const std::string& name, const mat4& m) const;
};