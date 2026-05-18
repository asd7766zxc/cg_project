#pragma once
#include "Vec.hpp"

class Camera {
public:

	float horizontal_sensitivity = 0.003;
	float vertical_sensitivity = 0.003;
	float fov = 90.0f;
	float nearp = .1f;
	float farp = 100.f;
	vec3 position = vec3(0, 0, 0);

	float rx, yx, rz;
	mat4 view, proj, cproj;

	vec3 vup = vec3(0,1,0);
	void updateView();
	void lookAt(vec3 focus);
	void mouseMove(float dx, float dy);
	void updateProj(int w, int h, float nearp, float farp, float fov);
	void make_ortho(int w, int h, float sz);
	void windowResize(int w, int h);
	vec3 getWorldMousePos(float mx, float my, int plane);
	mat4 getMatrix() const;
	vec3 getLookAt();
	//https://terathon.com/blog/oblique-clipping.html
	void ObliqueProj(vec3 pos, vec3 norm, bool clipOppo = false);
};