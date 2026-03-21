#include "ShaderProgram.hpp"
ShaderProgram::ShaderProgram(vector<std::shared_ptr<Shader>> shaders) {

	ID = glCreateProgram();
	for(auto shader : shaders) {
		glAttachShader(ID, shader->ID);
	}
	glLinkProgram(ID);
	checkCompileErrors();
}
void ShaderProgram::checkCompileErrors() {
	int success;
	char infoLog[512];
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "Shader Program Linking Error" << infoLog << std::endl;
	}
}
void ShaderProgram::use() {
	glUseProgram(ID);
}
void ShaderProgram::setFloat(const std::string& name, float v) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), v);
}
void ShaderProgram::setVec3(const std::string& name, vec3 v) const {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
}
void ShaderProgram::setInt(const std::string& name, int v) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), v);
}
void ShaderProgram::setMat4(const std::string& name, const mat4& m) const {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_TRUE, m.mt);
}