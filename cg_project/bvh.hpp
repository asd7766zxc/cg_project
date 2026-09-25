#pragma once
#include "aabb.hpp"
#include "Helpers.hpp"
#include <algorithm>
#define BVH_MIN_SIZE 10
// bounding volume is sphere
struct Sphere {
	vec3 center;
	float radius;

};
struct Patch {
	vec3 v[3];
	float getMinAlongAxis(int axis) const {
		return std::min({v[0][axis], v[1][axis], v[2][axis]});
	}
};
class bvh {
	aabb box;
	Sphere sphere;
	shared_ptr<bvh> left, right;
	//we separate the volume along the longest axis of BB.
	bvh(vector<int> patchIds, const vector<Patch>& patches) {
		for (auto& id : patchIds) {
			box.adjust(patches[id].v[0]);
			box.adjust(patches[id].v[1]);
			box.adjust(patches[id].v[2]);
		}
		if (patchIds.size() < BVH_MIN_SIZE) return;
		int axis = box.longestAxis();//0:x,1:y,2:z
		std::sort(patchIds.begin(), patchIds.end(), [&](int a, int b) {
			return patches[a].getMinAlongAxis(axis) < patches[b].getMinAlongAxis(b);
		});
		int m = (patchIds.size() - 1) / 2;
		left = make_shared<bvh>(vector<int>(patchIds.begin(), patchIds.begin() + m), patches);
		right = make_shared<bvh>(vector<int>(patchIds.begin() + m + 1, patchIds.end()), patches);

	}
};