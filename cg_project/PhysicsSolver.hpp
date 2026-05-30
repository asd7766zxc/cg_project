#pragma once
#include "GameObject.hpp"
#include "Voxelizer.hpp"
#include "CollisionDetector.hpp"
#include "ForceGenerator.hpp"
#include "CollisionResolver.hpp"
#include "WaterGrid.hpp"

class PhysicsSolver {
public:
	vector<shared_ptr<GameObject>> entity_list;
	shared_ptr<Voxelizer> voxelizer;
	shared_ptr<CollisionDetector> collision_detector;
	void add_entity(shared_ptr<GameObject> entity) {
		voxelizer->calculate_distance_field(entity);
		voxelizer->calculate_gravitycenter(entity);
		voxelizer->calculate_tensorOfInertia(entity);
		entity->update_aabb();
		entity_list.push_back(entity);
	}
	shared_ptr<Gravity> g;
	PhysicsSolver(shared_ptr<Voxelizer> voxelizer) : voxelizer(voxelizer) {
		collision_detector = make_shared<CollisionDetector>(voxelizer);
		g = make_shared<Gravity>(vec3(0, -9.8, 0));
	}
	void update(float dt,shared_ptr<WaterGrid> grid,bool paused) {
		if (paused) {
			for (auto& c : entity_list) c->update_aabb();
			//for (auto& c : entity_list) voxelizer->voxelize(c);
			collision_detector->update_grid(entity_list);

			collision_detector->collision_solve_regular(entity_list);
			return;
		}
		//add gravity
		for (auto& c : entity_list) {
			c->lastFrameAcceleration = vec3(0.0);
		}
		for (auto& c : entity_list) {
			g->updateForce(c, dt);
		}
		for (auto& c : entity_list) c->integrate(dt);
		for (auto& c : entity_list) c->update_aabb();
		//for (auto& c : entity_list) voxelizer->voxelize(c);
		collision_detector->update_grid(entity_list);
		
		collision_detector->collision_solve_regular(entity_list);
		//simple collision resolve
		for (auto& a : collision_detector->collisions) {
			if (a.inwater) {
				float displacedFluid = a.voxel_count * voxelizer->voxel_size * voxelizer->voxel_size * voxelizer->voxel_size;
				a.A->addForceAtPoint(-displacedFluid * a.B->density * g->gravity,a.A->buoyancy_center);
				auto grid_point = (grid->internal_object->localToWorld()).inverse() * a.point;
				grid->applyWaveAt(grid_point.x, grid_point.z,a.penetration,a.A);
				continue;
			}

			CollisionResolver::ResolveVelocity(a,dt);
			CollisionResolver::ResolvePenetration(a);
		}
	}
};