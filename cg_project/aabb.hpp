#pragma once
#include "interval.hpp"
#include "Helpers.hpp"
#include "ray.hpp"

class GameObject; //forward declaration
class aabb {
public:
	interval x, y, z;
	GameObject* ref_obj = nullptr;

	aabb() {}
	aabb(interval x, interval y, interval z) : x(x), y(y), z(z) {}
	aabb(aabb& a, aabb& b) : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}
	
	const interval& axis(int i) {
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}
	const interval& axis_interval(int i) const {
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}
	
	bool hit(const aabb& other) const {
		return x.intersect(other.x) && y.intersect(other.y) && z.intersect(other.z);
	}
	bool hit(const ray& r, interval& ray_tt) const {
		const vec3& ray_orig = r.origin();
		const vec3& ray_dir = r.direction();

		interval ray_t = ray_tt;
		for (int axis = 0; axis < 3; axis++) {
			const interval& ax = axis_interval(axis);
			const double adinv = 1.0 / ray_dir[axis];

			auto t0 = (ax.min - ray_orig[axis]) * adinv;
			auto t1 = (ax.max - ray_orig[axis]) * adinv;
			if (t0 > t1) std::swap(t0, t1);
			if (t0 > ray_t.min) ray_t.min = t0;
			if (t1 < ray_t.max) ray_t.max = t1;
			if (ray_t.max <= ray_t.min)
				return false;
		}
		ray_tt = ray_t;
		return true;
	}
	void reset() {
		x = interval();
		y = interval();
		z = interval();
	}
	int longestAxis() const {
		float x_size = x.size();
		float y_size = y.size();
		float z_size = z.size();
		if (x_size > y_size && x_size > z_size) return 0;
		else if (y_size > x_size && y_size > z_size) return 1;
		else return 2;
	}
};