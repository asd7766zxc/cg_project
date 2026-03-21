#include "Shader.hpp"

Shader::Shader(const char* source, GLuint shader_type) {
	std::ifstream shader_file;
	shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	std::string shader_code;

	try {
		
		shader_file.open(source);
		std::stringstream ss;
		ss << shader_file.rdbuf();
		shader_file.close();
		shader_code = ss.str();

	}catch (std::ifstream::failure& e) {
		std::cout << "Fail to read shader source : " << e.what() << std::endl;
	}

	ID = glCreateShader(shader_type);
	const char* shader_code_cstr = shader_code.c_str();
	glShaderSource(ID, 1, &shader_code_cstr, NULL);
	glCompileShader(ID);
	checkCompileErrors();
}

void Shader::checkCompileErrors() {
	int success;
	char infoLog[512];
	glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(ID, 512, NULL, infoLog);
		std::cout << "Shader Compilation Error" << infoLog << std::endl;
	}
}