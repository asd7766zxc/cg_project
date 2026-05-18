#pragma once
#include "Vec.hpp"
#include "Camera.hpp"
#include "Helpers.hpp"
#include <glad/glad.h>

class ReflectionTexture {
public:
	//world coord
	vec3 plane_u, plane_v, plane_o;
	GLuint reflectFBO;
	GLuint relfectTexture;
	ReflectionTexture(int width, int height) {
		
		glGenFramebuffers(1, &reflectFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);

		glGenTextures(1, &relfectTexture);
		glBindTexture(GL_TEXTURE_2D, relfectTexture);


		updateTexture(width,height);
	}
	void updateTexture(int width, int height) {
		glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);
		glBindTexture(GL_TEXTURE_2D, relfectTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, relfectTexture, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void render(shared_ptr<Camera> original_cam, auto draw_half,int width,int height) {
		updateTexture(width, height);
		auto u = plane_u;
		auto v = plane_v;

		vec3 normal = (u ^ v);
		normal = uni(normal);
		original_cam->ObliqueProj(plane_o, normal);
		std::swap(original_cam->proj, original_cam->cproj);

		glViewport(0, 0, width, height);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, reflectFBO);

		draw_half();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::swap(original_cam->proj, original_cam->cproj);
	}
};