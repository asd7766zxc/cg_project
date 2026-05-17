#pragma once
#include "CollisionDetector.hpp"

class CollisionResolver {
public:
	void resolve(contact_attribute contact) {
		
		auto cnormal = contact.normal;
		auto cpos = contact.point;
		auto A = contact.A;
		auto B = contact.B;
		auto contact_coord = mat4::trans(cpos) * mat4::axisAsX(cnormal); // to world
		auto to_contact = mat4::axisAsX(cnormal).transposed() * mat4::trans(-cpos);
		vec3 rA = cpos - A->getWorldGravityCenter(); //the r
		vec3 iTorqueA = rA ^ cnormal; //a unit impulse that generate impulsive torque
		vec3 delRotationA = A->inverseInertiaWorld() * iTorqueA;//that generate change of angular velocity
		vec3 iVelA = delRotationA ^ rA; //that gives amount of velocity change (a unit impluse that gives)

		auto iVelA_contact = to_contact * iVelA;
		iVelA_contact += A->inverseMass();

		if (!B->hasInifiniteMass()) {
				
		}
	}
};