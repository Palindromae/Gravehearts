#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "Physics_Component.h"

struct CollisionResponse {

};

/////////// Calculate if Collisions are Valid ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool CollisionPoint(const float timestep, const glm::vec3& ObjA_contact, const glm::vec3& ObjA_velocity, const glm::vec3& ObjB_contact, const glm::vec3& ObjB_velocity, glm::vec3& PointOfCollision) {
	glm::vec3 delta_position = ObjB_contact - ObjA_contact;
	glm::vec3 delta_velocity = (ObjB_velocity - ObjA_velocity) * timestep;

	PointOfCollision = delta_position / delta_velocity;
	PointOfCollision = PointOfCollision * ObjA_velocity + ObjA_contact;

	return glm::all(glm::lessThanEqual(delta_position, delta_velocity));
}


bool SeperatingAxiis() {
	return false;
}





/////////// Resolve Collisions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ResolveCollision(PhysicsComponent objA_ID, glm::vec3 objA_position, PhysicsComponent ObjB_ID, glm::vec3 objB_position, float distanceToPosition) {

	glm::vec3 A_rotation_arm = 

}
