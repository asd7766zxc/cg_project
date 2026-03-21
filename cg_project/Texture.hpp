#pragma once
#include "glad/glad.h"
#include "stb_image.h"
#include <iostream>
#include <string>

class Texture {
	public:
	unsigned int ID;
	int width, height, nrChannels;
	Texture(const std::string& image_source, GLenum internal_format = GL_RGB);
	Texture(int width, int height,int engravesize = 1);
	Texture();
	void bind() const;

};