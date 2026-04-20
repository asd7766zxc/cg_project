#pragma once
#include "interval.hpp"
#include "Helpers.hpp"

class GameObject; //forward declaration
class aabb {
public:
	interval x, y, z;
	shared_ptr<GameObject> ref_obj;

	aabb() {}
	aabb(interval x, interval y, interval z) : x(x), y(y), z(z) {}
	aabb(aabb& a, aabb& b) : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}
	
	const interval& axis(int i) {
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}
	
	bool hit(const aabb& other) const {
		return x.intersect(other.x) && y.intersect(other.y) && z.intersect(other.z);
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