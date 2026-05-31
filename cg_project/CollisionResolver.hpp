#pragma once
#include "CollisionDetector.hpp"

const float angularMovementLimitation = 0.002;
const float velocityLimitation = 0.2;
const float penetration_beta = 0.2f;
const float slop = 0.0001f;
class CollisionResolver {
public:
	static void ResolveVelocity(contact_attribute contact,float dt) {
		if (contact.inwater) return;
		// this resolution ensure one of two objects is movable
		if (contact.A->hasInifiniteMass()) {
			// swap A B 
			swap(contact.A, contact.B);
			std::swap(contact.data.Na, contact.data.Nb);
			std::swap(contact.data.Pa, contact.data.Pb);
			std::swap(contact.data.Sa, contact.data.Sb);

		}
		
		float restitution = 0.0f;

		auto cnormal = contact.data.Na; //the normal is A to B (change to (B to A))  // 假裝是 B 往 A 撞 (A's persepective)
		auto cpos = contact.data.Pa;
		auto A = contact.A;
		auto B = contact.B;

		auto contact_coord = mat4::axisAsX(cnormal); // to world
		auto to_contact = mat4::axisAsX(cnormal).transposed();

		auto getContactSpeedPerImpulse = [&](shared_ptr<GameObject> obj) {
			vec3 r = cpos - obj->getWorldGravityCenter(); // relateive contact position (force apply point)
			vec3 torqueByUnitImpulse = r ^ cnormal; //a unit impulse that generate impulsive torque
			vec3 rotationByUnitImpulse = obj->inverseInertiaWorld() * torqueByUnitImpulse;//that generate change of angular velocity
			vec3 velcoityByUnitImpulse = rotationByUnitImpulse ^ r; //that gives amount of velocity change (a unit impluse that gives)
			float iVel_contact = (to_contact * vec4(velcoityByUnitImpulse,0)).x;
			iVel_contact += obj->inverseMass();//linear
			return iVel_contact; // the velocity change in +x;
		};

		auto deltaVelocity = getContactSpeedPerImpulse(A);
		if (!B->hasInifiniteMass()) deltaVelocity += getContactSpeedPerImpulse(B);
		//deltaVelocity means if we apply 1 unit impulse, how much velocity change we will get in contact point (in contact coordinate)
		//beacuase impulse is apply to both object
		
		auto getLocalEnclosingVelocity = [&](shared_ptr<GameObject> obj) {
			vec3 r = cpos - obj->getWorldGravityCenter(); // relateive contact position (force apply point)
			return (to_contact * vec4(obj->velocity + (obj->rotation ^ r), 0)).x;
		};
		
		float velocityFromAcc = dt * (to_contact * A->lastFrameAcceleration).x;
		if (!B->hasInifiniteMass()) velocityFromAcc -= dt * (to_contact * B->lastFrameAcceleration).x;

		//vs' = -cvs => delVs = -(1 + e) * cv
		// -> -acc - (1 + e)(vs - vacc)
		float contact_velocity = getLocalEnclosingVelocity(A) - getLocalEnclosingVelocity(B); // A closing to B (-x)
		if (contact_velocity > 0.0f) return;
		if (std::fabs(contact_velocity) < velocityLimitation) {
			restitution = 0.0f; // if the velocity is very small, we treat it as in rest, no bounce
		}
		float desired_delta_velocity = -(1 + restitution) * contact_velocity; // temporary
		//float desired_delta_velocity = -contact_velocity - restitution * (contact_velocity - velocityFromAcc); // temporary

		vec3 contact_impulse = vec3(desired_delta_velocity / deltaVelocity,0,0);
		vec3 impulse = (contact_coord * vec4(contact_impulse, 0)).toVec3(); // world impulse // IMPORTATNT NO TRANSLATION  

		A->velocity += (impulse * A->inverseMass()); // linear
		A->rotation += A->inverseInertiaWorld() * ((cpos - A->getWorldGravityCenter()) ^ impulse); // angular
		if (!B->hasInifiniteMass()) {
			B->velocity -= (impulse * B->inverseMass());
			B->rotation -= B->inverseInertiaWorld() * ((cpos - B->getWorldGravityCenter()) ^ impulse);
		}
	}

	//nonlinear projection method
	//project the inertia in contact normal direction (the resistance of object to move)
	static void ResolvePenetration(contact_attribute contact) {
		if (contact.inwater) return;
		// this resolution ensure one of two objects is movable
		if (contact.A->hasInifiniteMass()) {
			// swap A B 
			swap(contact.A, contact.B);
			std::swap(contact.data.Na, contact.data.Nb);
			std::swap(contact.data.Pa, contact.data.Pb);
			std::swap(contact.data.Sa, contact.data.Sb);
		}


		auto cnormal = contact.data.Na; //the normal is A to B (change to (B to A))  // 假裝是 B 往 A 撞 (A's persepective)
		auto cpos = contact.data.Pa;
		auto A = contact.A;
		auto B = contact.B;
		float penetration = abs(contact.data.Pa - contact.data.Sa) * penetration_beta; // relaxation
		penetration = std::max(penetration - slop,0.0f);
		auto contact_coord = mat4::axisAsX(cnormal); // to world
		auto to_contact = mat4::axisAsX(cnormal).transposed();

		//use cnormal as unit movement 
		auto getContactNormalRotationInertia = [&](shared_ptr<GameObject> obj) {
			vec3 r = cpos - obj->getWorldGravityCenter(); // relateive contact position (movement apply point)
			vec3 inertiaWorld = r ^ cnormal; 
			inertiaWorld = obj->inverseInertiaWorld() * inertiaWorld;//that generate change of angular velocity
			inertiaWorld = inertiaWorld ^ r; //that gives amount of velocity change (a unit impluse that gives)
			float interialContact = (to_contact * vec4(inertiaWorld, 0)).x;
			return interialContact; // the movement change in +x;
		};

		auto angularInertiaA = getContactNormalRotationInertia(A);
		auto angularInertiaB = getContactNormalRotationInertia(B);

		auto linearInertiaA = A->inverseMass();
		auto linearInertiaB = B->inverseMass();

		float totalInertia = linearInertiaA + angularInertiaA + linearInertiaB + angularInertiaB;
		float inverseTotalInertia = 1.0f / totalInertia;
		auto angularMoveA = (penetration * angularInertiaA * inverseTotalInertia);
		auto angularMoveB = -(penetration * angularInertiaB * inverseTotalInertia);

		auto linearMoveA = (penetration * linearInertiaA * inverseTotalInertia);
		auto linearMoveB = -(penetration * linearInertiaB * inverseTotalInertia);
		auto correctAngularMove = [&](shared_ptr<GameObject> obj,float &angularMove,float &linearMove) {
			vec3 r = cpos - obj->getWorldGravityCenter();
			float limit = angularMovementLimitation * abs(r);
			if (std::fabs(angularMove) > limit) { //Correction by book
				float totalMove = angularMove + linearMove; //Extra contribute to linear
				if(angularMove >= limit) {
					angularMove = limit;
				}
				else {
					angularMove = -limit;
				}
				linearMove = totalMove - angularMove;
			}
		};

		correctAngularMove(A, angularMoveA, linearMoveA);
		correctAngularMove(B, angularMoveB, linearMoveB);

		//linear movement 
		A->position += cnormal * linearMoveA; // B to A
		if (!B->hasInifiniteMass()) {
			B->position += cnormal * linearMoveB;
		}

		// this is "Linear move" that caused by angular movement 

		//delRotation = I^-1 * (r x impulse)
		
		auto getRotationByAngularMove = [&](shared_ptr<GameObject> obj, float angularMove, float angularInteria) {
			vec3 r = cpos - obj->getWorldGravityCenter();
			vec3 rotationByUnitImpulse = obj->inverseInertiaWorld() * (r ^ cnormal);
			vec3 rotation = (rotationByUnitImpulse * (1 / angularInteria)) * angularMove; // the movement in contact normal direction
			return delta_rotation(obj->orientation,(rotation)); // the change in orientation
		};
		if (angularInertiaA > eps) {
			A->orientation += getRotationByAngularMove(A, angularMoveA, angularInertiaA);
		}
		if (!B->hasInifiniteMass() && angularInertiaB > eps) {
			B->orientation += getRotationByAngularMove(B, angularMoveB, angularInertiaB);
		}
	}
};