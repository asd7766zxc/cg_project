#include "Helpers.hpp"


std::mt19937 mt;
float random_float() {
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(mt);
}