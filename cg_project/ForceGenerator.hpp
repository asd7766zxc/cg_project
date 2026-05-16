#pragma once
#include "GameObject.hpp"

class ForceGenerator {
public:
	virtual void updateForce(shared_ptr<GameObject> a, float dt) = 0;
};
class Gravity : ForceGenerator {
public:
	vec3 gravity;
	Gravity(vec3 gravity) : gravity(gravity) {}
	void updateForce(shared_ptr<GameObject> a, float dt) override {
		if (a->hasInifiniteMass()) return;
		a->addForce(gravity * a->mass);
	}
};
class Spring : ForceGenerator {
public:
	vec3 connect_point;
	vec3 connect_other;

	shared_ptr<GameObject> other;
	float k; //hooks law
	float restLength; //L0
	Spring(vec3 connect_point,vec3 connect_other, shared_ptr<GameObject> other,float k,float restLength):\
		connect_point(connect_point), connect_other(connect_other), other(other), k(k), restLength(restLength) {
	}

	void updateForce(shared_ptr<GameObject> a, float dt) override {
		vec3 iws = a->toWorld() * connect_point;
		vec3 ows = other->toWorld() * connect_other;
		vec3 force = iws - ows;
		float m = abs(force);
		m = std::fabs(m - restLength);
		m *= k; // f = kdx
		force = uni(force);
		force *= -m;
		a->addForceAtPoint(force, iws);
	}
};

class Buoyance : ForceGenerator {
public:
	//Buoyance(float displaceFluid, ) : level(level) {} // volume below this level will get buoyancy force
	
};