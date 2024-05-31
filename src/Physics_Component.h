#pragma once
#include <glm/glm.hpp>


enum class ColliderType {
	Model,  // Has a physical model to test against
	Virtual // Has no model backing it
};

struct PhysicsComponent 
{
	
	
	
	// Gravity
	bool ApplyGravity{};
	float Gravity = 10;
	
	// Collisions 
	bool Collisions_BodyBody{};
	bool Collisions_BodyWorld{};
	


	// Filters
	uint32_t mask = 0;

	// Modifiers
	float SharpnessVector = 0; // 0 is blunt otherwise calculate the direction from bits

	// Rotation
	glm::vec3 CentreOfRotation{};
	float MomentOfInertia{};
	float InverseInertia{};

	// General Physics 
	float mass = 0;
	float inverse_mass = 0;

	// Friction
	float StaticFriction{};
	float DynamicFriction{};

	// CPU Colliders
	glm::vec3 origin{};
	glm::vec3 dimensions{};
	ColliderType collderType{};

	PhysicsComponent() {
		ApplyGravity = false;
		Collisions_BodyBody = false;
		Collisions_BodyWorld = false;
	}

	// Create a component using all programmatically defined values. This should be set on a the model being used
	PhysicsComponent(glm::vec3 origin, glm::vec3 dimension, float mass = 1, uint32_t mask = 0xff) :  dimension(dimension), mask(mask), mass(mass){
		inverse_mass = (mass != 0) ? 1 / mass : 0;
		CentreOfRotation = dimension / glm::vec3(2.0); // assume that the CoM is in the middle. 
	}

	bool PhysicsActive() {
		return ApplyGravity || Collisions_BodyBody || Collisions_BodyWorld;
	}

};