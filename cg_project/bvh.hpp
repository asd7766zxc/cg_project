#pragma once
#include "aabb.hpp"
#include "Helpers.hpp"

class bvh {
	aabb box;
	shared_ptr<bvh> left, right;
	bool hit() {
	}
};