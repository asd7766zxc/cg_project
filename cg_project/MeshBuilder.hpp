#pragma once
#include "Model.hpp"
#include "Helpers.hpp"
class MeshBuilder {
public:
	const float cube_vertices[8 * 36] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,

		 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
		 -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		 -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,

		 -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

		  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 
		  0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 

		  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 
		  0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 

		  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
		   0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
		   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,

		   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		  -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
		  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

		   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		   0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		  -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,

		  -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		  -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
		   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f 
	};
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
		//auto to_inv_cartesian = [&](int i, int j) {
		//	float angle = 2 * i * pi / step;
		//	float phi = j * pi / step;
		//	float z = 0.9 * std::sin(phi) * std::cos(angle);
		//	float x = 0.9 * std::sin(phi) * std::sin(angle);
		//	float y = 0.9 * std::cos(phi);

		//	// Position
		//	vertices.push_back(x);
		//	vertices.push_back(y);
		//	vertices.push_back(z);

		//	//Normal
		//	vertices.push_back(-(x / 0.9));
		//	vertices.push_back(-(y / 0.9));
		//	vertices.push_back(-(z / 0.9));

		//	//Texture Coordinate
		//	vertices.push_back(float(i) / step); // u
		//	vertices.push_back(float(j) / step); //v
		//	};
		for (int i = 0; i < step; ++i) {
			for (int j = 0; j < step; ++j) {
				to_cartesian(i, j);
				to_cartesian(i, (j + 1) % (step + 1));
				to_cartesian((i + 1) % (step + 1), j);

				to_cartesian((i + 1) % (step + 1), j);
				to_cartesian(i, (j + 1) % (step + 1));
				to_cartesian((i + 1) % (step + 1), (j + 1) % (step + 1));

			}
		}
		// sphere inner 
		/*for (int i = 0; i < step; ++i) {
			for (int j = 0; j < step; ++j) {
				to_inv_cartesian(i, j);
				to_inv_cartesian((i + 1) % (step + 1), j);
				to_inv_cartesian(i, (j + 1) % (step + 1));

				to_inv_cartesian(i + 1, j);
				to_inv_cartesian((i + 1) % (step + 1), (j + 1) % (step + 1));
				to_inv_cartesian(i, (j + 1) % (step + 1));
			}
		}*/
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

	// toward +z with unit lengh and diameter 0.5
	shared_ptr<Model> buildRod(float step) {
		vector<float> vertices;
		float r = 0.25;
		auto add_zero = [&](float z,int flat) {
			vertices.push_back(0);
			vertices.push_back(0);
			vertices.push_back(z);

			vertices.push_back(0);
			vertices.push_back(0);
			vertices.push_back(flat);

			vertices.push_back(0);
			vertices.push_back(0);
		};
		auto add = [&](int i, float z,int flat) {
			float angle = 2 * i * pi / step;
			
			float x = r * std::cos(angle);
			float y = r * std::sin(angle);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			if (flat) {
				vertices.push_back(0);
				vertices.push_back(0);
				vertices.push_back(flat);
			}
			else {
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(0);
			}

			vertices.push_back(0);
			vertices.push_back(0);
		};
		for (int i = 0; i < step; ++i) {
			add(i, 0, 0);
			add(i + 1, 0, 0);
			add(i + 1, 1.0f, 0);

			add(i + 1, 1.0f, 0);
			add(i, 1.0f, 0);
			add(i, 0.0f, 0);

			add(i, 0, -1);
			add(i + 1, 0, -1);
			add_zero(0, -1);

			add(i, 1, 1);
			add(i + 1, 1, 1);
			add_zero(1, 1);
		}

		return make_shared<Model>(vertices.data(), vertices.size() * sizeof(float), vertices.size() / 8);
	}
	shared_ptr<Model> buildArrow(float step,float length, float head_size) {
		vector<float> vertices;
		
		float r = 0.25;

		auto add_zero = [&](float z, int flat) {
			vertices.push_back(0);
			vertices.push_back(0);
			vertices.push_back(z);

			vertices.push_back(0);
			vertices.push_back(0);
			vertices.push_back(flat);

			vertices.push_back(0);
			vertices.push_back(0);
			};
		auto add = [&](int i, float z, int flat) {
			float angle = 2 * i * pi / step;

			float x = r * std::cos(angle);
			float y = r * std::sin(angle);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			if (flat == 2) {
				vec3 n(x, y, 1);
				n = uni(n);
				vertices.push_back(n.x);
				vertices.push_back(n.y);
				vertices.push_back(n.z);
			}
			else if (flat) {
				vertices.push_back(0);
				vertices.push_back(0);
				vertices.push_back(flat);
			}
			else {
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(0);
			}

			vertices.push_back(0);
			vertices.push_back(0);
			};
		for (int i = 0; i < step; ++i) {
			r = 0.25;
			add(i, 0, 0);
			add(i + 1, 0, 0);
			add(i + 1, 1.0f * length, 0);

			add(i + 1, 1.0f * length, 0);
			add(i, 1.0f * length, 0);
			add(i, 0.0f, 0);

			add(i, 0, -1);
			add(i + 1, 0, -1);
			add_zero(0, -1);

			r = 0.5 * head_size;
			add(i, 1 * length, 1);
			add(i + 1, 1 * length, 1);
			add_zero(1 * length, 1);

			add(i, 1 * length, 2);
			add(i + 1, 1 * length, 2);
			r = 0;
			add(i, 1 * length + 0.5 * head_size, 2);

		}

		return make_shared<Model>(vertices.data(), vertices.size() * sizeof(float), vertices.size() / 8);
	}

	shared_ptr<Model> buildCone(float step,float head_size) {
		vector<float> vertices;

		float r = 0.25;

		auto add_zero = [&](float z, int flat) {
			vertices.push_back(0);
			vertices.push_back(0);
			vertices.push_back(z);

			vertices.push_back(0);
			vertices.push_back(0);
			vertices.push_back(flat);

			vertices.push_back(0);
			vertices.push_back(0);
			};
		auto add = [&](int i, float z, int flat) {
			float angle = 2 * i * pi / step;

			float x = r * std::cos(angle);
			float y = r * std::sin(angle);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			if (flat == 2) {
				vec3 n(x, y, 1);
				n = uni(n);
				vertices.push_back(n.x);
				vertices.push_back(n.y);
				vertices.push_back(n.z);
			}
			else if (flat) {
				vertices.push_back(0);
				vertices.push_back(0);
				vertices.push_back(flat);
			}
			else {
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(0);
			}

			vertices.push_back(0);
			vertices.push_back(0);
			};
		float length = 0;
		for (int i = 0; i < step; ++i) {
			
			r = 0.5 * head_size;
			add(i, 1 * length, 1);
			add(i + 1, 1 * length, 1);
			add_zero(1 * length, 1);

			add(i, 1 * length, 2);
			add(i + 1, 1 * length, 2);
			r = 0;
			add(i, 1 * length + 0.5 * head_size, 2);

		}

		return make_shared<Model>(vertices.data(), vertices.size() * sizeof(float), vertices.size() / 8);
	}


	shared_ptr<Model> buildCube() {
		return make_shared<Model>((float*)cube_vertices, sizeof(cube_vertices), 36);
	}
	static shared_ptr<Model> Sphere(int step) { MeshBuilder mb; return mb.buildSphere(step); };
	static shared_ptr<Model> Plane(float step, vec3 u, vec3 v, vec3 o) { MeshBuilder mb; return mb.buildPlane(step,u,v,o); };
	static shared_ptr<Model> Cube() { MeshBuilder mb; return mb.buildCube(); }
	static shared_ptr<Model> Rod(int step) { MeshBuilder mb; return mb.buildRod(step); }
	static shared_ptr<Model> Arrow(int step,float length,float head_size = 1.0f) { MeshBuilder mb; return mb.buildArrow(step, length, head_size); }
	static shared_ptr<Model> Cone(int step, float head_size = 1.0f) { MeshBuilder mb; return mb.buildCone(step, head_size); }
};