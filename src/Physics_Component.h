#pragma once
#include <glm/glm.hpp>

struct PhysicsComponent 
{
	
	
	
	// Gravity
	bool ApplyGravity;
	float Gravity = 10;
	
	// Collisions 
	bool Collisions_BodyBody;
	bool Collisions_BodyWorld;
	
	// Collider - This might need to be ripped out and placed into a buffer
	//glm::vec3 origin{}; We're going to use the body as the hitbox, since a collider cant be larger than the truth box
	glm::vec3 dimension{};

	// Filters
	uint32_t mask = 0;

	// Modifiers
	float SharpnessVector = 0; // 0 is blunt otherwise calculate the direction from bits

	// General Physics 
	glm::vec3 Velocity{};
	glm::vec4 Angular_Velocity{};

	PhysicsComponent() {
		ApplyGravity = false;
		Collisions_BodyBody = false;
		Collisions_BodyWorld = false;
	}

	// Create a component using all
	PhysicsComponent(glm::vec3 origin, glm::vec3 dimension, uint32_t mask = 0xff) : origin(origin), dimension(dimension), mask(mask){

	}

	bool PhysicsActive() {
		return ApplyGravity || Collisions_BodyBody || Collisions_BodyWorld;
	}

};