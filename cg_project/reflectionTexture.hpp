#include "Vec.hpp"
#include "Camera.hpp"
class reflectionTexture {
	vec3 plane_u, plane_v, plane_o;

	void render(const Camera& original_cam, auto& draw_world) {
		vec3 normal = (u ^ v);
		normal = uni(normal);
		original_cam.ObliqueProj(o, normal);
	}
};