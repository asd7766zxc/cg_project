#pragma once
#undef min
#undef max
class interval {
public:
	float min, max;
	interval() : min(1e38F),max(-1.0e38F){}
	interval(float min, float max) : min(min), max(max) {}
	interval(interval& a, interval& b) {
		min = std::min(a.min, b.min);
		max = std::max(a.max, b.max);
	}
	const float& get(int i) const {
		if (i == 0) return min;
		return max;
	}
	void adjust(float v) {
		max = std::max(max, v);
		min = std::min(min, v);
	}
	float size() const {
		return max - min;
	}

	bool intersect(const interval& other) const {
		return !(max < other.min || min > other.max);
	}
};