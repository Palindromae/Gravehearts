#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "Physics_Component.h"
#include "../shaders/Physics_RayCollision.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


/////////// Calculate if Collisions are Valid ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Calculate if the collision happens and where, For Model on X collisions
bool CollisionPoint(const float timestep, const glm::vec3 delta_velocity, const glm::vec3& ObjA_contact, const glm::vec3& ObjA_velocity, const glm::vec3& ObjB_contact, glm::vec3& PointOfCollision);



struct SATCollision {
    glm::vec3 MinimumTranslationVector{};
    float ADist{};
    float BDist{};
    float timeToTarget{};
    int A_ID;
    int B_ID;

    // To Be filled in during position resolution
    glm::vec3 PointOfContact;
};

glm::vec3 FindMinTranslationVector(glm::vec3 vector, glm::vec3 minimumVector, const float BorderProject, const float VelocityProject, const float CurrentSeperatingDistance, float& MinSeperatingDistance, const bool currentSeperation,  float& timeToTarget);

bool SeperationTest(const float BorderProject, const float VelocityProject, const float CurrentSeperatingDistance);


bool SATOptimised(const glm::vec3* PositionsBuffer, const glm::quat* RotationBuffer, const int objA, glm::vec3 ASize, int objB, glm::vec3 BSize, SATCollision& collisionInfo, glm::vec3 DeltaVelocity);

void UpdateAngularVelocity(float deltaTime, uint32_t ID, glm::quat* AngularVelocity, glm::quat* RotationBuffer, glm::quat* RotationBuffer_Past);

void UpdateVelocity(float deltaTime, uint32_t ID, glm::vec3* VelocityBuffer, glm::vec3* PositionBuffer, glm::vec3* PositionBuffer_Past);

/////////// Resolve Collisions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Calculate the closing velocity. Total energy of a physics object in a frame
glm::vec3 ClosingVelocity(glm::vec3 Velocity, glm::quat AngularVelocity, glm::vec3 rotationArm);

void ApplyForce(float deltaTime, glm::vec3 force, const uint32_t ID, glm::vec3* VelocityBuffer);

bool ResolvePositionAfterCollision(const float deltaTime, glm::vec3& contact_point, const glm::vec3 ABDeltaPositionResolution, const glm::vec3 normal, const PhysicsComponent& objA, const uint32_t objA_id, const PhysicsComponent& objB, const uint32_t objB_id, glm::vec3* PositionBuffer, glm::vec3* VelocityBuffer);


void ResolvePositionAfterCollision_World(const PhysicsRayCollision collision, const PhysicsComponent& objA, const uint32_t objA_id, glm::vec3* PositionBuffer, glm::vec3* VelocityBuffer);


void ResolveVelocitiesAfterCollision(const float deltaTime, float distanceToPosition, glm::vec3 contact_point, glm::vec3 normal,
	glm::vec3* PositionBuffer, glm::quat* RotationBuffer, glm::vec3* VelocityBuffer, glm::quat* AngularVelocityBuffer, 
	glm::vec3* PositionBuffer_Past, glm::quat* RotationBuffer_Past, glm::vec3* VelocityBuffer_Past, glm::quat* AngularVelocityBuffer_Past,
	const PhysicsComponent& objA, uint32_t objA_id,
	const PhysicsComponent& objB, uint32_t objB_id);


void ResolveVelocitiesAfterCollision_World(const float deltaTime, float distanceToPosition, glm::vec3 contact_point, glm::vec3 normal,
    glm::vec3* PositionBuffer, glm::quat* RotationBuffer, glm::vec3* VelocityBuffer, glm::quat* AngularVelocityBuffer,
    glm::vec3* PositionBuffer_Past, glm::quat* RotationBuffer_Past, glm::vec3* VelocityBuffer_Past, glm::quat* AngularVelocityBuffer_Past,
    const PhysicsComponent& objA, uint32_t objA_id, glm::vec3 objA_position_collisionPoint);

//float frictionFraction = glm::dot(delta_velocity, normal);
//
//ApplyForce(frictionFraction, delta_velocity, objA_id, VelocityBuffer);
//ApplyForce(frictionFraction, -delta_velocity, objB_id, VelocityBuffer);