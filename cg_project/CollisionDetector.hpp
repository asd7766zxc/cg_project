#pragma once
#include "GameObject.hpp"
#include "Voxelizer.hpp"
#include "Vec.hpp"

// the point and normal will be in object A, so push the b toward normal
struct contact_attribute {
	shared_ptr<GameObject> A, B;
	vec3 normal;
	vec3 point;
	float penetration = 0.0f;
	float min_penetration;
	int voxel_count = 0;
	bool inwater = false;
	bool draw_onB = false;
};
class CollisionDetector {
public:
	struct collision_attribute {
		int point_x = 0;
		int point_y = 0;
		int point_z = 0;

		int normal_x = 0;
		int normal_y = 0;
		int normal_z = 0;

		int voxel_count = 0;
	};
	struct collision_distance {
		int max_penetration = 0;
		int min_penetration = 0;
	};
	static const unsigned int REGULAR_DIVISION = 8;
	const int regular_div = 21 / (REGULAR_DIVISION - 1);
	vector<shared_ptr<GameObject>> regular_grid[REGULAR_DIVISION + 1][REGULAR_DIVISION + 1][REGULAR_DIVISION + 1];
	void clear_grid() {
		for (auto& c : regular_grid)
			for (auto& b : c)
				for (auto& d : b) {
					d.clear();
				}
	}
	void update_grid(const vector<shared_ptr<GameObject>>& entity_list) {
		clear_grid();
		for (auto& a : entity_list) {
			auto& bb = a->bounding_box;
			int mnx = bb->x.min / regular_div;
			int mxx = bb->x.max / regular_div;

			int mny = bb->y.min / regular_div;
			int mxy = bb->y.max / regular_div;

			int mnz = bb->z.min / regular_div;
			int mxz = bb->z.max / regular_div;

			for (int x = mnx; x <= mxx; ++x) {
				for (int y = mny; y <= mxy; ++y) {
					for (int z = mnz; z <= mxz; ++z) {
						if (x >= REGULAR_DIVISION || x < 0) continue;
						if (y >= REGULAR_DIVISION || y < 0) continue;
						if (z >= REGULAR_DIVISION || z < 0) continue;

						regular_grid[x][y][z].push_back(a);
					}
				}
			}
		}
	}
	shared_ptr<ShaderProgram> collision_program;
	shared_ptr<ShaderProgram> collision_attribute_program;
	shared_ptr<Voxelizer> voxelizer;
	vector<contact_attribute> collisions;
	GLuint ssbo[2];
	CollisionDetector(shared_ptr<Voxelizer> voxelizer) : voxelizer(voxelizer) {
		collision_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("collision.comp", GL_COMPUTE_SHADER)
		});

		collision_attribute_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("collision_attribute.comp", GL_COMPUTE_SHADER)
		});
		glGenBuffers(1, &ssbo[0]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[0]);

		glGenBuffers(1, &ssbo[1]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[1]);
	}

	collision_attribute zero;
	collision_distance zero1;
	void resolve_collision(shared_ptr<GameObject> a, shared_ptr<GameObject> b) {
		if (a->penetrable && b->penetrable) return;
		
		bool waterflag = (a->penetrable || b->penetrable);

		if (a->penetrable) swap(a, b); // always let b be water;
		if (!b->penetrable && b->hasInifiniteMass()) swap(a, b);

		voxelizer->voxelize(a);
		voxelizer->voxelize(b);

		glBindImageTexture(0, a->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
		glBindImageTexture(1, b->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

		//TODO : find the aabb enclosing the result and using this boundary to speed up
		bool draw_result_onB = false;
		if (a->voxel_info.voxel_count > b->voxel_info.voxel_count) draw_result_onB = true;
		glBindImageTexture(2, draw_result_onB ? b->collisionVisualizeTexture : a->collisionVisualizeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

		collision_program->use();
		collision_program->setInt("draw_result_onB", draw_result_onB);
		collision_program->setInt("voxelInfoA.dimx", a->voxel_info.dim_x);
		collision_program->setInt("voxelInfoA.dimy", a->voxel_info.dim_y);
		collision_program->setInt("voxelInfoA.dimz", a->voxel_info.dim_z);

		collision_program->setInt("voxelInfoB.dimx", b->voxel_info.dim_x);
		collision_program->setInt("voxelInfoB.dimy", b->voxel_info.dim_y);
		collision_program->setInt("voxelInfoB.dimz", b->voxel_info.dim_z);

		collision_program->setInt("voxelInfoA.corner_x", a->voxel_info.corner_x);
		collision_program->setInt("voxelInfoA.corner_y", a->voxel_info.corner_y);
		collision_program->setInt("voxelInfoA.corner_z", a->voxel_info.corner_z);

		collision_program->setInt("voxelInfoB.corner_x", b->voxel_info.corner_x);
		collision_program->setInt("voxelInfoB.corner_y", b->voxel_info.corner_y);
		collision_program->setInt("voxelInfoB.corner_z", b->voxel_info.corner_z);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(collision_attribute), &zero, GL_DYNAMIC_READ);

		glDispatchCompute((a->voxel_info.dim_x + 7) / 8, (a->voxel_info.dim_y + 7) / 8, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		collision_attribute* attributes = (collision_attribute*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		if (attributes->voxel_count == 0) return;
		// integer points are corners need to move to center
		vec3 collision_p = vec3(attributes->point_x, attributes->point_y, attributes->point_z) * (1.0 / attributes->voxel_count) * voxelizer->voxel_size + a->voxel_info.box_corner + vec3(voxelizer->voxel_size / 2.0);
		vec3 contact_normal = vec3(attributes->normal_x, attributes->normal_y, attributes->normal_z);
		if (!attributes->normal_x && !attributes->normal_y && !attributes->normal_z) return;
		bool inverse_normal = false;
		if ((a->getWorldGravityCenter() - collision_p) * uni(contact_normal) > 0) {
			contact_normal *= -1;
			attributes->normal_x *= -1;
			attributes->normal_y *= -1;
			attributes->normal_z *= -1;
		}
		//std::cout << attributes->normal_x;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(collision_distance), &zero1, GL_DYNAMIC_READ);

		collision_attribute_program->use();
		collision_attribute_program->setInt("voxelInfoA.dimx", a->voxel_info.dim_x);
		collision_attribute_program->setInt("voxelInfoA.dimy", a->voxel_info.dim_y);
		collision_attribute_program->setInt("voxelInfoA.dimz", a->voxel_info.dim_z);

		collision_attribute_program->setInt("voxelInfoB.dimx", b->voxel_info.dim_x);
		collision_attribute_program->setInt("voxelInfoB.dimy", b->voxel_info.dim_y);
		collision_attribute_program->setInt("voxelInfoB.dimz", b->voxel_info.dim_z);

		collision_attribute_program->setInt("voxelInfoA.corner_x", a->voxel_info.corner_x);
		collision_attribute_program->setInt("voxelInfoA.corner_y", a->voxel_info.corner_y);
		collision_attribute_program->setInt("voxelInfoA.corner_z", a->voxel_info.corner_z);

		collision_attribute_program->setInt("voxelInfoB.corner_x", b->voxel_info.corner_x);
		collision_attribute_program->setInt("voxelInfoB.corner_y", b->voxel_info.corner_y);
		collision_attribute_program->setInt("voxelInfoB.corner_z", b->voxel_info.corner_z);

		glDispatchCompute((a->voxel_info.dim_x + 7) / 8, (a->voxel_info.dim_y + 7) / 8, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		collision_distance* distances = (collision_distance*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		float l = abs(contact_normal);

		float coef = a->voxel_info.voxel_size * (1.0 / l) * (1.0 / attributes->voxel_count);

		float real_max_penetration = coef * distances->max_penetration;
		float real_min_penetration = coef * distances->min_penetration;
		vec3 min_contact_normal = uni(contact_normal) * real_min_penetration;
		
		a->buoyancy_center = collision_p;
		
		collision_p += min_contact_normal;
		float penetration = real_max_penetration - real_min_penetration;

		
		// add contact records
		collisions.push_back({
			a,b,
			uni(contact_normal),
			collision_p,
			penetration,
			real_min_penetration,
			attributes->voxel_count,
			waterflag,
			draw_result_onB,
		});
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	};

	void collision_solve_regular(const vector<shared_ptr<GameObject>>& entity_list) {
		collisions.clear();
		for (auto& a : entity_list) {
			vector<shared_ptr<GameObject>> hitlist;

			auto& bb = a->bounding_box;
			int mnx = bb->x.min / regular_div;
			int mxx = bb->x.max / regular_div;

			int mny = bb->y.min / regular_div;
			int mxy = bb->y.max / regular_div;

			int mnz = bb->z.min / regular_div;
			int mxz = bb->z.max / regular_div;

			for (int x = mnx; x <= mxx; ++x) {
				for (int y = mny; y <= mxy; ++y) {
					for (int z = mnz; z <= mxz; ++z) {
						if (x >= REGULAR_DIVISION || x < 0) continue;
						if (y >= REGULAR_DIVISION || y < 0) continue;
						if (z >= REGULAR_DIVISION || z < 0) continue;
						for (auto& b : regular_grid[x][y][z]) {
							if (a == b) continue; // hit self
							if (a > b) continue; // resolved
							if (a->bounding_box->hit(*b->bounding_box)) {
								hitlist.push_back(b);
								//fast check by using AABB
							}
						}
					}
				}
			}
			std::sort(hitlist.begin(), hitlist.end());
			hitlist.erase(std::unique(hitlist.begin(), hitlist.end()), hitlist.end());
			for (auto& b : hitlist) {
				// futher check by using voxelization result
				resolve_collision(a, b);
			}
		}
	};
};