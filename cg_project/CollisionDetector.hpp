#pragma once
#include "GameObject.hpp"
#include "Voxelizer.hpp"
#include "Vec.hpp"

// the point and normal will be in object A, so push the b toward normal
struct df_data {
	vec3 cp;
	
	vec3 Pa;
	vec3 Pb;

	vec3 Na;
	vec3 Nb;

	vec3 Sa;
	vec3 Sb;

	vec3 collide;

	float accumulated_time = 0.0f;
};
struct contact_attribute {
	shared_ptr<GameObject> A, B;
	vec3 normal;
	vec3 point;
	float penetration = 0.0f;
	float min_penetration;
	float max_penetration;
	int voxel_count = 0;
	bool inwater = false;
	bool draw_onB = false;
	df_data data;
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

		int nx = INT_MAX, mx = INT_MIN;
		int ny = INT_MAX, my = INT_MIN;
		int nz = INT_MAX, mz = INT_MIN;
	};

	struct collision_split {
		int point_x[8] = {0};
		int point_y[8] = {0};
		int point_z[8] = {0};

		int normal_x[8] = {0};
		int normal_y[8] = {0};
		int normal_z[8] = {0};

		int voxel_count[8] = {0};
	};
	struct collision_distance {
		int max_penetration = 0;
		int min_penetration = 0;
	};
	Camera camera;
	static const unsigned int REGULAR_DIVISION = 20;
	static const int voxel_splitting_limit = 100;
	const float scence_size = 50.0f;
	const float offset = scence_size;
	const int regular_div = (scence_size * 2) / (REGULAR_DIVISION - 1);
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
			int mnx = (bb->x.min + offset) / regular_div;
			int mxx = (bb->x.max + offset) / regular_div;

			int mny = (bb->y.min + offset) / regular_div;
			int mxy = (bb->y.max + offset) / regular_div;

			int mnz = (bb->z.min + offset) / regular_div;
			int mxz = (bb->z.max + offset) / regular_div;


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
	shared_ptr<ShaderProgram> collision_splitting_program;
	shared_ptr<ShaderProgram> distance_program;
	shared_ptr<ShaderProgram> distance_field_program;
	shared_ptr<ShaderProgram> ray_voxel_program;

	shared_ptr<Voxelizer> voxelizer;
	vector<contact_attribute> collisions;
	GLuint ssbo[4];
	GLuint ray_inersect_buffer;
	GLuint distance_map,distance_fbo;
	const int raterizer_resolution = 100;
	void bind_buffers() {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo[0]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo[1]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo[2]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo[3]);
	}
	CollisionDetector(shared_ptr<Voxelizer> voxelizer) : voxelizer(voxelizer) {
		collision_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("collision.comp", GL_COMPUTE_SHADER)
		});

		collision_attribute_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("collision_attribute.comp", GL_COMPUTE_SHADER)
		});

		collision_splitting_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("voxel_split_center.comp", GL_COMPUTE_SHADER)
		});
		distance_field_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("distance_field_gradient.comp", GL_COMPUTE_SHADER)
		});
		ray_voxel_program = make_shared<ShaderProgram>
			(vector<shared_ptr<Shader>>{
			make_shared<Shader>("ray_voxel_intersect.comp", GL_COMPUTE_SHADER)
		});

		glGenBuffers(1, &ssbo[0]);
		glGenBuffers(1, &ssbo[1]);
		glGenBuffers(1, &ssbo[2]);
		glGenBuffers(1, &ssbo[3]);
		glGenBuffers(1, &ray_inersect_buffer);

		glGenTextures(1, &distance_map);
		glBindTexture(GL_TEXTURE_2D, distance_map);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(distance_map, 0, GL_DEPTH_COMPONENT, raterizer_resolution, raterizer_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glGenFramebuffers(1, &distance_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, distance_fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, distance_fbo, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	collision_attribute zero;
	collision_distance zero1;
	collision_split zeros;
	collision_attribute buffer[8];
	int descent_iterations = 100;
	float descent_step = 0.0005f;
	void resolve_collision(shared_ptr<GameObject> a, shared_ptr<GameObject> b, float dt) {
		if (a->penetrable && b->penetrable) return;
		if (a->hasInifiniteMass() && b->hasInifiniteMass()) return;
		
		bind_buffers();
		bool waterflag = (a->penetrable || b->penetrable);

		if (a->penetrable) swap(a, b); // always let b be water;
		else if (!a->penetrable && a->hasInifiniteMass()) swap(a, b); // always let be the stasis one

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
		collision_attribute att = *attributes;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		if (attributes->voxel_count == 0) return;
		if (attributes->voxel_count > voxel_splitting_limit && false) {
			int currentx = att.point_x / att.voxel_count;
			int currenty = att.point_y / att.voxel_count;
			int currentz = att.point_z / att.voxel_count;

			collision_splitting_program->use();
			collision_splitting_program->setInt("voxelInfoA.dimx", a->voxel_info.dim_x);
			collision_splitting_program->setInt("voxelInfoA.dimy", a->voxel_info.dim_y);
			collision_splitting_program->setInt("voxelInfoA.dimz", a->voxel_info.dim_z);

			collision_splitting_program->setInt("voxelInfoA.corner_x", a->voxel_info.corner_x);
			collision_splitting_program->setInt("voxelInfoA.corner_y", a->voxel_info.corner_y);
			collision_splitting_program->setInt("voxelInfoA.corner_z", a->voxel_info.corner_z);


			collision_splitting_program->setInt("currentCenterA.x", currentx);
			collision_splitting_program->setInt("currentCenterA.y", currenty);
			collision_splitting_program->setInt("currentCenterA.z", currentz);


			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(zeros), &zeros, GL_DYNAMIC_READ);
			
			glDispatchCompute((a->voxel_info.dim_x + 7) / 8, (a->voxel_info.dim_y + 7) / 8, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

			collision_split* buffer_tmp = (collision_split*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

			for (int i = 0; i < 8; ++i) {
				buffer[i].point_x = buffer_tmp->point_x[i];
				buffer[i].point_y = buffer_tmp->point_y[i];
				buffer[i].point_z = buffer_tmp->point_z[i];

				buffer[i].normal_x = buffer_tmp->normal_x[i];
				buffer[i].normal_y = buffer_tmp->normal_y[i];
				buffer[i].normal_z = buffer_tmp->normal_z[i];

				buffer[i].voxel_count = buffer_tmp->voxel_count[i];
			}

			collision_attribute_program->use();

			collision_attribute_program->setInt("currentCenterA.x", currentx);
			collision_attribute_program->setInt("currentCenterA.y", currenty);
			collision_attribute_program->setInt("currentCenterA.z", currentz);
			for (int i = 0; i < 8; ++i) {
				spliting(a, b, &buffer[i], waterflag, draw_result_onB,i+1,dt);
			}
		}
		else {
			spliting(a, b, &att,waterflag,draw_result_onB,0,dt);
		}
	};
	struct ray_data {
		vec3 direct;
		vec3 ori;
		float l;
		float r;
		unsigned mutex = 0u;
		int hit = 0;
	};
	ray_data null_data;
	bool intersect_with_ray(shared_ptr<GameObject> a, const ray& r, interval& ray_tt) {
		ray_voxel_program->use();
		null_data.mutex = 0u;
		null_data.hit = 0;
		null_data.ori = r.origin();
		null_data.direct = r.direction();
		null_data.l = ray_tt.min;
		null_data.r = ray_tt.max;
		ray_voxel_program->setInt("voxel_size_p", voxelizer->voxel_size_p);
		ray_voxel_program->setInt("voxel_size_q", voxelizer->voxel_size_q);
		ray_voxel_program->setInt("voxelInfoA.dimx", a->voxel_info.dim_x);
		ray_voxel_program->setInt("voxelInfoA.dimy", a->voxel_info.dim_y);
		ray_voxel_program->setInt("voxelInfoA.dimz", a->voxel_info.dim_z);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_inersect_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ray_data), &null_data, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ray_inersect_buffer);
		glBindImageTexture(0, a->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
		glDispatchCompute((a->voxel_info.dim_x + 7) / 8, (a->voxel_info.dim_y + 7) / 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		ray_data* data = (ray_data*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		bool hit = data->hit;
		if (data->hit) {
			ray_tt.min = data->l;
			ray_tt.max = data->r;
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return hit;
	}

	df_data bisection_method(shared_ptr<GameObject> a, shared_ptr<GameObject> b, vec3 collision_p,float t, float dt) {
		aabb boxA = a->getInterpolatedAABB(t, dt);
		aabb boxB = b->getInterpolatedAABB(t, dt);
		if(!boxA.hit(boxB)){
			df_data data;
			data.collide = vec3(100.0f);
			return data;
		}
		mat4 FtoA = a->model->fieldToMesh();
		mat4 FtoB = b->model->fieldToMesh();

		mat4 AtoF = FtoA.inverse();
		mat4 BtoF = FtoB.inverse();

		mat4 AtoW = a->interpolate_localToWorld(t,dt);
		mat4 BtoW = b->interpolate_localToWorld(t,dt);

		mat4 WtoA = AtoW.inverse();
		mat4 WtoB = BtoW.inverse();

		// working in A's LCS
		collision_p = (AtoF * WtoA * vec4(collision_p, 1.0f)).toVec3();
		//from A's field to B's field
		mat4 AtoB = BtoF * WtoB * AtoW * FtoA;
		mat4 AtoBlocal = WtoB * AtoW * FtoA;
		distance_field_program->use();
		distance_field_program->setMat4("AtoB", AtoB);
		distance_field_program->setFloat("BtoA_scaling", (b->model->box_size * b->scale.x) / (a->model->box_size * a->scale.x));
		distance_field_program->setFloat("epsilon", 1e-4);
		distance_field_program->setFloat("step_size", descent_step);
		distance_field_program->setVec3("initialPoint", collision_p);
		distance_field_program->setInt("max_interation", descent_iterations);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, a->distanceTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, b->distanceTexture);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(df_data), NULL, GL_DYNAMIC_READ);

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		df_data* data = (df_data*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (data->collide.x > 10.0f) {
			return *data;
		}
		data->Na = uni((AtoW * FtoA * vec4(data->Na, 0)).toVec3());
		data->Nb = uni((BtoW * AtoBlocal * vec4(data->Nb, 0)).toVec3());

		data->cp = (AtoW * FtoA * vec4(data->cp, 1)).toVec3();

		data->Pa = (FtoA * vec4(data->Pa, 1)).toVec3();
		data->Pb = (AtoBlocal * vec4(data->Pb, 1)).toVec3();


		data->Sa = (FtoA * vec4(data->Sa, 1)).toVec3();
		data->Sb = (AtoBlocal * vec4(data->Sb, 1)).toVec3();
		return *data;
	}

	float target_precision = 1e-5f;
	// 000000111111
	df_data contact_generate_bisection_distance_field(shared_ptr<GameObject> a, shared_ptr<GameObject> b, vec3 collision_p, float dt) {
		float l = 0, r = 1.0f;
		df_data data; 
		while (r - l > target_precision) {
			float m = (r + l) / 2;
			data = bisection_method(a, b, collision_p, m, dt);
			if (data.collide.x > 10.0f) l = m; 
			else r = m; //collide
		}
		data = bisection_method(a, b, collision_p, r, dt);
		if (data.collide.x > 10.0f) {
			return data;
		}
		a->adjustTo(r,dt);
		b->adjustTo(r,dt);
		data.accumulated_time = 1.0f - r;
		return data;
	}
	df_data contact_generate_distance_field(shared_ptr<GameObject> a, shared_ptr<GameObject> b ,vec3 collision_p, float dt) {
		// working in A's LCS
		collision_p = (a->model->meshToField() * a->worldToLocal() * vec4(collision_p, 1.0f)).toVec3();
		//from A's field to B's field
		mat4 AtoB = b->model->meshToField() * b->worldToLocal() * a->localToWorld() * a->model->fieldToMesh();
		mat4 AtoBlocal = b->worldToLocal() * a->localToWorld() * a->model->fieldToMesh();
		distance_field_program->use();
		distance_field_program->setMat4("AtoB", AtoB);
		distance_field_program->setFloat("BtoA_scaling", (b->model->box_size * b->scale.x) / (a->model->box_size * a->scale.x));
		distance_field_program->setFloat("epsilon",1e-4);
		distance_field_program->setFloat("step_size", descent_step);
		distance_field_program->setVec3("initialPoint", collision_p);
		distance_field_program->setInt("max_interation", descent_iterations);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, a->distanceTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, b->distanceTexture);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[3]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(df_data), NULL, GL_DYNAMIC_READ);

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		df_data* data = (df_data*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (data->collide.x > 10.0f) {
			return *data;
		}
		auto toLocal = a->model->fieldToMesh();
		data->Na = uni((a->localToWorld () * toLocal * vec4(data->Na, 0)).toVec3());
		data->Nb = uni((b->localToWorld () * AtoBlocal * vec4(data->Nb, 0)).toVec3());
		
		data->cp = (a->localToWorld() * toLocal * vec4(data->cp, 1)).toVec3();
		
		data->Pa = (toLocal * vec4(data->Pa, 1)).toVec3();
		data->Pb = (AtoBlocal * vec4(data->Pb, 1)).toVec3();


		data->Sa = (toLocal * vec4(data->Sa, 1)).toVec3();
		data->Sb = (AtoBlocal * vec4(data->Sb, 1)).toVec3();
		data->accumulated_time = 0.0f;
		auto tmp = *data;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		return tmp;

	}
	void spliting(shared_ptr<GameObject> a, shared_ptr<GameObject> b, collision_attribute *attributes,bool waterflag,bool draw_result_onB,int index,float dt) {
		// integer points are corners need to move to center
		vec3 collision_p = vec3(attributes->point_x, attributes->point_y, attributes->point_z) * (1.0 / attributes->voxel_count) * voxelizer->voxel_size + a->voxel_info.box_corner + vec3(voxelizer->voxel_size / 2.0);
		vec3 contact_normal = vec3(attributes->normal_x, attributes->normal_y, attributes->normal_z);
		if (!attributes->normal_x && !attributes->normal_y && !attributes->normal_z && !waterflag) return;
		bool inverse_normal = false;
		if ((a->getWorldGravityCenter() - collision_p) * uni(contact_normal) > 0) {
			contact_normal *= -1;
			attributes->normal_x *= -1;
			attributes->normal_y *= -1;
			attributes->normal_z *= -1;
		}
		df_data data;
		if (!waterflag) {
			data = contact_generate_distance_field(a, b , collision_p, dt);
			if (data.collide.x > 10.0f) {
				return;
			}
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(collision_distance), &zero1, GL_DYNAMIC_READ);

		collision_attribute_program->use();
		collision_attribute_program->setInt("splitting", index);
		collision_attribute_program->setInt("voxelInfoA.dimx", a->voxel_info.dim_x);
		collision_attribute_program->setInt("voxelInfoA.dimy", a->voxel_info.dim_y);
		collision_attribute_program->setInt("voxelInfoA.dimz", a->voxel_info.dim_z);

		collision_attribute_program->setInt("voxelInfoA.corner_x", a->voxel_info.corner_x);
		collision_attribute_program->setInt("voxelInfoA.corner_y", a->voxel_info.corner_y);
		collision_attribute_program->setInt("voxelInfoA.corner_z", a->voxel_info.corner_z);


		glDispatchCompute((a->voxel_info.dim_x + 7) / 8, (a->voxel_info.dim_y + 7) / 8, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		collision_distance* distances = (collision_distance*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		float l = abs(contact_normal);
		float coef = a->voxel_info.voxel_size * (1.0 / l) * (1.0 / attributes->voxel_count);
		if (index) coef /= 8.0f;

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
			real_max_penetration,
			attributes->voxel_count,
			waterflag,
			draw_result_onB,
			data,
		});
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	void collision_solve_regular(const vector<shared_ptr<GameObject>>& entity_list,float dt) {
		collisions.clear();
		for (auto& a : entity_list) {
			vector<shared_ptr<GameObject>> hitlist;

			auto& bb = a->bounding_box;
			int mnx = (bb->x.min+offset) / regular_div;
			int mxx = (bb->x.max+offset) / regular_div;

			int mny = (bb->y.min+offset) / regular_div;
			int mxy = (bb->y.max+offset) / regular_div;

			int mnz = (bb->z.min+offset) / regular_div;
			int mxz = (bb->z.max+offset) / regular_div;

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
				resolve_collision(a, b, dt);
			}
		}
	};
	
};