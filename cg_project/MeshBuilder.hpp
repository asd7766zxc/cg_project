#pragma once
#include "Model.hpp"
#include "Helpers.hpp"
class MeshBuilder {
public:
	shared_ptr<Model> buildSphere(int step) {
		vector<float> vertices;
		auto to_cartesian = [&](int i,int j) {
			float angle = 2 * i * pi / step;
			float phi = j * pi / step;
			float z = std::sin(phi) * std::cos(angle);
			float x = std::sin(phi) * std::sin(angle);
			float y = std::cos(phi);

			// Position
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			//Normal
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			//Texture Coordinate
			vertices.push_back(float(i) / step); // u
			vertices.push_back(float(j) / step); //v
		};
		auto to_inv_cartesian = [&](int i, int j) {
			float angle = 2 * i * pi / step;
			float phi = j * pi / step;
			float z = 0.9 * std::sin(phi) * std::cos(angle);
			float x = 0.9 * std::sin(phi) * std::sin(angle);
			float y = 0.9 * std::cos(phi);

			// Position
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			//Normal
			vertices.push_back(-(x / 0.9));
			vertices.push_back(-(y / 0.9));
			vertices.push_back(-(z / 0.9));

			//Texture Coordinate
			vertices.push_back(float(i) / step); // u
			vertices.push_back(float(j) / step); //v
			};
		for (int i = 0; i < step; ++i) {
			for (int j = 0; j < step; ++j) {
				to_cartesian(i, j);
				to_cartesian((i + 1) % (step + 1), j);
				to_cartesian(i, (j + 1) % (step + 1));

				to_cartesian(i + 1, j);
				to_cartesian((i + 1) % (step + 1), (j + 1) % (step + 1));
				to_cartesian(i, (j + 1) % (step + 1));

			}
		}
		for (int i = 0; i < step; ++i) {
			for (int j = 0; j < step; ++j) {
				to_inv_cartesian(i, j);
				to_inv_cartesian((i + 1) % (step + 1), j);
				to_inv_cartesian(i, (j + 1) % (step + 1));

				to_inv_cartesian(i + 1, j);
				to_inv_cartesian((i + 1) % (step + 1), (j + 1) % (step + 1));
				to_inv_cartesian(i, (j + 1) % (step + 1));
			}
		}
		return make_shared<Model>(vertices.data(), vertices.size() * sizeof(float), vertices.size() / 8);
	}

	shared_ptr<Model> buildPlane(float step, vec3 u, vec3 v, vec3 o) {
		vector<float> vertices;
		vec3 normal = uni(u ^ v);
		auto add_vertex = [&](int i, int j) {
			float s = float(i) / step;
			float t = float(j) / step;
			vec3 pos = o + u * i + v * j;
			vertices.push_back(pos.x);
			vertices.push_back(pos.y);
			vertices.push_back(pos.z);
			vertices.push_back(normal.x);
			vertices.push_back(normal.y);
			vertices.push_back(normal.z);
			vertices.push_back(s); // u
			vertices.push_back(t); // v
		};
		for (int i = 0; i < step; ++i) {
			for (int j = 0; j < step; ++j) {
				add_vertex(i, j);
				add_vertex(i + 1, j);
				add_vertex(i, j + 1);

				add_vertex(i + 1, j);
				add_vertex(i, j + 1);
				add_vertex(i + 1, j + 1);
			}
		}
		return make_shared<Model>(vertices.data(), vertices.size() * sizeof(float), vertices.size() / 8);
	}
};