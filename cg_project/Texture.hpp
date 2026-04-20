#pragma once
#include "glad/glad.h"
#include "stb_image.h"
#include <iostream>
#include <string>
#include "Vec.hpp"

class Texture {
	public:
	unsigned int ID;
	int width, height, nrChannels;
	Texture(const std::string& image_source, GLenum internal_format = GL_RGB);
	Texture(int width, int height,int engravesize = 1);
	Texture(int width, int height, vec3 colorA, vec3 colorB);
	Texture();
	void bind() const;

};