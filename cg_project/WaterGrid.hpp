#pragma once
#include "GameObject.hpp"

const int max_resolution = 1000;
class WaterGrid{
public:
	shared_ptr<GameObject> internal_object;
	shared_ptr<Model> grid_surface;
	
	int grid_resolution = 100; //[0,1] x [0,1] x [0,1]
	int padding = 0;
	// surface 
	float height_map_old[max_resolution][max_resolution];
	float height_map[max_resolution][max_resolution];
	float height_map_new[max_resolution][max_resolution];
	float vertices[max_resolution * max_resolution * 8 * 6]; // position + normal + texcoord
	float base_water_height = 0.4f;
	int draw_resolution = 0;
	void updateGrid() {
		int t = 0;
		draw_resolution = grid_resolution - padding * 2;
		float delta = 1.0f / (draw_resolution - 1);
		auto add_vertices = [&](int i, int j,int side = 0,vec3 norm = vec3(0.0)) {
			float x = float(i) * delta;
			float z = float(j) * delta;
			float y = height_map[i + padding][j + padding] + base_water_height;
			// Position
			vertices[t + 0] = (x);
			vertices[t + 1] = (side ? 0 : (y));
			vertices[t + 2] = (z);
			// y = f(x,z)
			// n = (-df/dx, 1, -df/dz) 
			// = (- (f(x+dx,z) - f(x-dx,z)) / 2dx, 1, - (f(x,z+dz) - f(x,z-dz)) / 2dz)
			if (!side) {
				if(0 < i && i < draw_resolution - 1 && 0 < j && j < draw_resolution - 1) {
					float dfdx = height_map[i + 1 + padding][j + padding] - height_map[i - 1 + padding][j + padding];
					float dfdz = height_map[i + padding][j + 1 + padding] - height_map[i + padding][j - 1 + padding];

					dfdx /= 2 * delta;
					dfdz /= 2 * delta;

					vec3 normal = uni(vec3(-dfdx, 1, -dfdz));
					vertices[t + 3] = normal.x;
					vertices[t + 4] = normal.y;
					vertices[t + 5] = normal.z;
				}
				else {
					vertices[t + 3] = 0;
					vertices[t + 4] = 1;
					vertices[t + 5] = 0;
				}
			}
			else {
				vertices[t + 3] = norm.x;
				vertices[t + 4] = norm.y;
				vertices[t + 5] = norm.z;
			}

			vertices[t + 6] = (x); // u
			vertices[t + 7] = (z); //v

			t += 8;
		};
		for (int i = 0; i < draw_resolution - 1; ++i) {
			for (int j = 0; j < draw_resolution - 1; ++j) {
				add_vertices(i ,j);
				add_vertices(i + 1, j);
				add_vertices(i + 1, j + 1);

				add_vertices(i, j);
				add_vertices(i, j + 1);
				add_vertices(i + 1, j + 1);
			}
		}

		vec3 norm = vec3(-1, 0, 0);
		for (int j = 0; j < draw_resolution - 1; ++j) {
			add_vertices(0, j, 0, norm);
			add_vertices(0, j + 1, 0, norm);
			add_vertices(0, j + 1, 1, norm);

			add_vertices(0, j, 0, norm);
			add_vertices(0, j, 1, norm);
			add_vertices(0, j + 1, 1, norm);
		}
		norm = vec3(1, 0, 0);
		for (int j = 0; j < draw_resolution - 1; ++j) {
			add_vertices(draw_resolution - 1, j, 0, norm);
			add_vertices(draw_resolution - 1, j + 1, 0, norm);
			add_vertices(draw_resolution - 1, j + 1, 1, norm);

			add_vertices(draw_resolution - 1, j, 0, norm);
			add_vertices(draw_resolution - 1, j, 1, norm);
			add_vertices(draw_resolution - 1, j + 1, 1, norm);
		}

		norm = vec3(0, 0, 1);
		for (int j = 0; j < draw_resolution - 1; ++j) {
			add_vertices(j, draw_resolution - 1, 0, norm);
			add_vertices(j + 1, draw_resolution - 1, 0, norm);
			add_vertices(j + 1, draw_resolution - 1, 1, norm);

			add_vertices(j, draw_resolution - 1, 0, norm);
			add_vertices(j, draw_resolution - 1, 1, norm);
			add_vertices(j + 1, draw_resolution - 1, 1, norm);
		}


		norm = vec3(0, 0, -1);
		for (int j = 0; j < draw_resolution - 1; ++j) {
			add_vertices(j,0, 0, norm);
			add_vertices(j + 1,0, 0, norm);
			add_vertices(j + 1,0, 1, norm);

			add_vertices(j,0, 0, norm);
			add_vertices(j,0, 1, norm);
			add_vertices(j + 1,0, 1, norm);
		}

		grid_surface = make_shared<Model>(vertices, sizeof(float) * t, t / 8);
	}
	int old_resolution;
	void initialization() {
		old_resolution = grid_resolution;
		float xc = 0.5f;
		float w = 0.05;
		float delta = 1.0f / (grid_resolution - 1);
		for (int i = 0; i < grid_resolution; ++i) {
			for (int j = 0; j < grid_resolution; ++j) {
				height_map_old[i][j] = height_map[i][j] = 0.0f; //initial height
				float x = float(i) * delta;
				float z = float(j) * delta;
				//height_map_old[i][j] = std::exp(-(x - xc) * (x - xc) / (w * w)) * std::exp(-(z - xc) * (z - xc) / (w * w)) * 0.5; // initial disturbance
			}
		}
	}
	WaterGrid(int grid_resolution, vec3 size, shared_ptr<Texture> texture,int padding = 10) : grid_resolution(grid_resolution),padding(padding){
		initialization();
		updateGrid();

		internal_object = make_shared<GameObject>(grid_surface, texture);
		internal_object->model = grid_surface;
		internal_object->scale = size;
		internal_object->penetrable = true;
		internal_object->mass = -1.0f; // infinite mass
		internal_object->density = 997.0f;
		internal_object->draw_without_physics = true;
		internal_object->update_aabb();
	}
	float damping = 0.99f;
	bool first = true;
	bool boundary_cancelation = true;
	float wave_speed = 1.0f;
	//leap frog algorithm
	void updateHeightMap(float dt) {
		float delta = 1.0f / (grid_resolution - 1);
		auto& hn = height_map_new;
		auto& ho = height_map_old;
		auto& h = height_map;
		auto c2 = wave_speed* wave_speed;
		float coef = dt * dt / (delta * delta);
		int bd = (grid_resolution - 2);
		for(int i = 0; i < grid_resolution; ++i) {
			for(int j = 0; j < grid_resolution; ++j) {
				if (0 < i && i < grid_resolution - 1 && 0 < j && j < grid_resolution - 1) {
					if (first) {
						h[i][j] = ho[i][j] + 0.5 * c2 * coef * (ho[i - 1][j] + ho[i + 1][j] - 2 * ho[i][j]) + \
							0.5 * c2 * coef * (ho[i][j - 1] + ho[i][j + 1] - 2 * ho[i][j]);
					}
					else {
						hn[i][j] = -ho[i][j] + 2 * h[i][j] + c2 * coef * (h[i - 1][j] + h[i + 1][j] - 2 * h[i][j]) + \
							c2 * coef * (h[i][j - 1] + h[i][j + 1] - 2 * h[i][j]);
						hn[i][j] *= damping;
					}
				}
			}
		}
		if (first) {
			first = false;
			return;
		}
		for (int i = 0; i < grid_resolution; ++i) {
			for (int j = 0; j < grid_resolution; ++j) {
				if (0 < i && i < grid_resolution - 1 && 0 < j && j < grid_resolution - 1) {
					ho[i][j] = h[i][j];
					h[i][j] = hn[i][j];
					h[i][j] *= damping;
				}
				else {
					ho[i][j] = h[i][j] = 0.0f; // boundary condition
				}
			}
		}
	}
	void update() {
		if (old_resolution != grid_resolution) initialization();
		float delta = 1.0f / (grid_resolution - 1);
		//Courant¡VFriedrichs¡VLewy condition
		float dt = (0.707 * delta / wave_speed);
		updateHeightMap(dt);
		updateGrid();

		internal_object->model = grid_surface;
		internal_object->update_aabb();
	}
	bool once = false;
	float wave_limit = 0.02f;
	float wave_amplifier = 0.05;
	void applyWaveAt(float xc,float zc,float p,shared_ptr<GameObject> a) {
		if (old_resolution != grid_resolution) initialization();
		if (isnan(xc) || isnan(zc)) return;
		float delta = 1.0f / (grid_resolution - 1);
		float w = 0.05;
		float dwave = std::min(0.02f, wave_amplifier *std::fabs(p - a->lastAppliedWave));
		a->lastAppliedWave = p;
		if (dwave < eps) return;
		//std::cout << "apply wave at " << xc << "," << zc << " with power " << dwave << std::endl;
		for (int i = 0; i < grid_resolution; ++i) {
			for (int j = 0; j < grid_resolution; ++j) {
				float x = float(i) * delta;
				float z = float(j) * delta;
				height_map_old[i][j] += std::exp(-(x - xc) * (x - xc) / (w * w)) * std::exp(-(z - zc) * (z - zc) / (w * w)) * dwave; // initial disturbance
			}
		}
	}
};
