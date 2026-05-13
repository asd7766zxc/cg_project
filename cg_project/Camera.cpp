#include "Camera.hpp"

void Camera::updateView() {
	view = mat4::Rz(rz) * mat4::Rx(-rx) * mat4::Ry(yx) * mat4::trans(-position);
}
void Camera::lookAt(vec3 focus) {
	vec3 w = -(focus - position);
	vec3 u = vup ^ w;
	vec3 v = w ^ u;
	w = uni(w);
	u = uni(u);
	v = uni(v);
	view = mat4::coord(u,v,w) * mat4::trans(-position);
}
void Camera::mouseMove(float dx, float dy) {
	rx += dy * vertical_sensitivity;
	yx += dx * horizontal_sensitivity;
	if (rx >=  pi / 2) rx =  pi / 2 - 0.1f;
	if (rx <= -pi / 2) rx = -pi / 2 + 0.1f;
	updateView();
}

void Camera::updateProj(int w, int h, float nearp, float farp, float fov) {
	const float e = 1.0f / std::tan(fov * pi / 360.f);
	const float a = float(h) / float(w);
	const float d = nearp - farp;

	proj.makeZero();

	proj[0] = e * a;
	proj[5] = e;
	proj[10] = (farp + nearp) / d;
	proj[11] = (2 * farp * nearp) / d;
	proj[14] = -1.0f;
}

void Camera::make_ortho(int w, int h, float sz = 10) {
	float aspect_ratio = float(h) / float(w);
	float nearp = -0.0f;
	float farp = 100.0;
	float left = -sz;
	float right = sz;
	float bottom = -sz * aspect_ratio;
	float top = sz * aspect_ratio;
	proj.makeZero();
	proj[3] = -(right + left) / (right - left);
	proj[7] = -(top + bottom) / (top - bottom);
	proj[11] = -(farp + nearp) / (farp - nearp);

	proj[0] = 2 / (right - left);
	proj[5] = 2 / (top - bottom);
	proj[10] = -2 / (farp - nearp);
	proj[15] = 1;
	//std::cout << proj << '\n';
	return;
}

void Camera::windowResize(int w, int h) {
	updateProj(w, h, nearp, farp, fov);
}
mat4 Camera::getMatrix() const {
	return proj * view;
}

vec3 Camera::getLookAt() {
	return (view * vec4(0, 0, -1, 0)).toVec3();
}