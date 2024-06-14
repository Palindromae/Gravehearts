#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "Physics_Component.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
struct CollisionResponse {

};

/////////// Calculate if Collisions are Valid ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Calculate if the collision happens and where, For Model on X collisions
bool CollisionPoint(const float timestep, const glm::vec3 delta_velocity, const glm::vec3& ObjA_contact, const glm::vec3& ObjA_velocity, const glm::vec3& ObjB_contact, glm::vec3& PointOfCollision) {
	glm::vec3 delta_position = ObjB_contact - ObjA_contact;

	PointOfCollision = delta_position / delta_velocity;
	PointOfCollision = PointOfCollision * ObjA_velocity + ObjA_contact;

	return glm::all(glm::lessThanEqual(delta_position, delta_velocity));
}



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

glm::vec3 FindMinTranslationVector(glm::vec3 vector, glm::vec3 minimumVector, const float BorderProject, const float VelocityProject, const float CurrentSeperatingDistance, float& MinSeperatingDistance, const bool currentSeperation,  float& timeToTarget)
{
    if (vector ==  glm::vec3(0))
        return minimumVector;

    float effectOnVelocity = glm::max(0.0f, abs(BorderProject) - CurrentSeperatingDistance);

    float distanceToEdge = abs(BorderProject + glm::sign(VelocityProject) * (glm::min(effectOnVelocity, abs(VelocityProject))));
    if (!currentSeperation && abs(CurrentSeperatingDistance - abs(distanceToEdge)) < MinSeperatingDistance)
    {
        MinSeperatingDistance = glm::max(0.0f, abs(CurrentSeperatingDistance - abs(distanceToEdge)));//)
        timeToTarget = BorderProject / VelocityProject;
        return vector * -glm::sign(BorderProject) * (CurrentSeperatingDistance - abs(BorderProject));//-abs(VelocityProject)
    }
    return minimumVector;
}

bool SeperationTest(const float BorderProject, const float VelocityProject, const float CurrentSeperatingDistance)
{
    float duelProjection = 0;

    if (BorderProject < 0)
        duelProjection = glm::min(0.0f, BorderProject + VelocityProject);
    else
        duelProjection = glm::max(0.0f, BorderProject + VelocityProject);

    return glm::max(0.0f, abs(duelProjection)) > CurrentSeperatingDistance;
}


bool SATOptimised(const glm::vec3* PositionsBuffer, const glm::quat* RotationBuffer, const int objA, glm::vec3 ASize, int objB, glm::vec3 BSize, SATCollision& collisionInfo, glm::vec3 DeltaVelocity)
{
    ASize /= 2.0f;
    BSize /= 2.0f;

    glm::vec3 APos = PositionsBuffer[objA] + ASize;
    glm::vec3 BPos = PositionsBuffer[objB] + BSize;

    glm::vec3 ay = RotationBuffer[objA] * glm::vec3(0, 1, 0);
    glm::vec3 by = RotationBuffer[objB] * glm::vec3(0, 1, 0);

    glm::vec3 ax = RotationBuffer[objA] * glm::vec3(1, 0, 0);
    glm::vec3 bx = RotationBuffer[objB] * glm::vec3(1, 0, 0);

    glm::vec3 az = RotationBuffer[objA] * glm::vec3(0, 0, 1);
    glm::vec3 bz = RotationBuffer[objB] * glm::vec3(0, 0, 1);
    glm::vec3 T = BPos - APos;
    collisionInfo = {};
    bool seperated = false;
    bool currentSeperation;
    float BorderProject;
    float VelocityProject;
    float MinSeperatingDistance = 999999;
    float CurrentSeperatingDistance;



    //AX
    collisionInfo.ADist = ASize.x;
    collisionInfo.BDist = abs(BSize.x * dot(ax, bx)) + abs(BSize.y * dot(ax, by)) + abs(BSize.z * dot(ax, bz));
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ax);
    VelocityProject = dot(DeltaVelocity, ax);
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;

    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(ax, collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AY
    collisionInfo.ADist = ASize.y;
    collisionInfo.BDist = abs(BSize.x * dot(ay, bx)) + abs(BSize.y * dot(ay, by)) + abs(BSize.z * dot(ay, bz));

    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ay);
    VelocityProject = dot(DeltaVelocity, ay);
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(ay, collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AZ
    collisionInfo.ADist = ASize.z;
    collisionInfo.BDist = abs(BSize.x * dot(az, bx)) + abs(BSize.y * dot(az, by)) + abs(BSize.z * dot(az, bz));
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, az);
    VelocityProject = dot(DeltaVelocity, az);
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(az, collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);


    //BX
    collisionInfo.ADist = abs(ASize.x * dot(ax, bx)) + abs(ASize.y * dot(ax, by)) + abs(ASize.z * dot(ax, bz));
    collisionInfo.BDist = BSize.x;
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, bx);
    VelocityProject = dot(DeltaVelocity, bx);
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(bx, collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //BY
    collisionInfo.ADist = abs(ASize.x * dot(ay, bx)) + abs(ASize.y * dot(ay, by)) + abs(ASize.z * dot(ay, bz));
    collisionInfo.BDist = BSize.y;
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, by);
    VelocityProject = dot(DeltaVelocity, by);
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(by, collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //BZ
    collisionInfo.ADist = abs(ASize.x * dot(az, bx)) + abs(ASize.y * dot(az, by)) + abs(ASize.z * dot(az, bz));
    collisionInfo.BDist = BSize.z;
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, bz);
    VelocityProject = dot(DeltaVelocity, bz);
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(bz, collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    if (seperated)
        return true;


    float rxx = dot(ax, bx);
    float rzx = dot(az, bx);
    float rxz = dot(ax, bz);
    float ryx = dot(ay, bx);
    float rzy = dot(az, by);
    float rxy = dot(ax, by);
    float ryy = dot(ay, by);
    float ryz = dot(ay, bz);
    float rzz = dot(az, bz);


    //AX X BX
    collisionInfo.ADist = abs(ASize.x * rzx) + abs(ASize.z * ryx);
    collisionInfo.BDist = abs(BSize.x * rxz) + abs(BSize.z * rxy);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, az) * ryx - dot(T, ay) * rzx;
    VelocityProject = dot(DeltaVelocity, az) * ryx - dot(DeltaVelocity, ay) * rzx;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(ax, bx), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);
    //AX X BY
    collisionInfo.ADist = abs(ASize.y * rzy) + abs(ASize.z * ryy);
    collisionInfo.BDist = abs(BSize.x * rxz) + abs(BSize.z * rxx);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, az) * ryy - dot(T, ay) * rzy;
    VelocityProject = dot(DeltaVelocity, az) * ryy - dot(DeltaVelocity, ay) * rzy;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(ax, by), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AX X BZ
    collisionInfo.ADist = abs(ASize.y * rzz) + abs(ASize.z * ryz);
    collisionInfo.BDist = abs(BSize.x * rxy) + abs(BSize.y * rxx);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, az) * ryz - dot(T, ay) * rzz;
    VelocityProject = dot(DeltaVelocity, az) * ryz - dot(DeltaVelocity, ay) * rzz;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(ax, bz), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AY X BX
    collisionInfo.ADist = abs(ASize.x * rzx) + abs(ASize.z * rxx);
    collisionInfo.BDist = abs(BSize.y * ryz) + abs(BSize.z * ryy);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ax) * rzy - dot(T, az) * rxx;
    VelocityProject = dot(DeltaVelocity, ax) * rzy - dot(DeltaVelocity, az) * rxx;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(ay, bx), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AY X BY
    collisionInfo.ADist = abs(ASize.x * rxy) + abs(ASize.z * rzy);
    collisionInfo.BDist = abs(BSize.x * ryz) + abs(BSize.z * ryx);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ax) * rzy - dot(T, az) * rxy;
    VelocityProject = dot(DeltaVelocity, ax) * rzy - dot(DeltaVelocity, az) * rxy;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(ay, by), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AY X BZ
    collisionInfo.ADist = abs(ASize.x * rzz) + abs(ASize.z * rxz);
    collisionInfo.BDist = abs(BSize.x * ryy) + abs(BSize.z * ryx);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ax) * rzz - dot(T, az) * rxz;
    VelocityProject = dot(DeltaVelocity, ax) * rzz - dot(DeltaVelocity, az) * rxz;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(ay, bz), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AZ X BX
    collisionInfo.ADist = abs(ASize.x * ryx) + abs(ASize.y * rxx);
    collisionInfo.BDist = abs(BSize.y * rzz) + abs(BSize.z * rzy);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ay) * rxx - dot(T, ax) * ryx;
    VelocityProject = dot(DeltaVelocity, ay) * rxx - dot(DeltaVelocity, ax) * ryx;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(az, bx), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AZ X BY
    collisionInfo.ADist = abs(ASize.x * ryy) + abs(ASize.y * rxy);
    collisionInfo.BDist = abs(BSize.x * rzz) + abs(BSize.z * rzx);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ay) * rxy - dot(T, ax) * ryy;
    VelocityProject = dot(DeltaVelocity, ay) * rxy - dot(DeltaVelocity, ax) * ryy;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(az, by), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    //AZ X BZ
    collisionInfo.ADist = abs(ASize.x * ryz) + abs(ASize.y * rxz);
    collisionInfo.BDist = abs(BSize.x * rzy) + abs(BSize.y * rzx);
    CurrentSeperatingDistance = collisionInfo.ADist + collisionInfo.BDist;
    BorderProject = dot(T, ay) * rxz - dot(T, ax) * ryz;
    VelocityProject = dot(DeltaVelocity, ay) * rxz - dot(DeltaVelocity, ax) * ryz;
    currentSeperation = SeperationTest(BorderProject, VelocityProject, CurrentSeperatingDistance);
    seperated |= currentSeperation;
    collisionInfo.MinimumTranslationVector = FindMinTranslationVector(glm::cross(az, bz), collisionInfo.MinimumTranslationVector, BorderProject, VelocityProject, CurrentSeperatingDistance, MinSeperatingDistance, currentSeperation, collisionInfo.timeToTarget);

    return seperated;

}

void UpdateAngularVelocity(float deltaTime, uint32_t ID, glm::quat* AngularVelocity, glm::quat* RotationBuffer, glm::quat* RotationBuffer_Past) {
	AngularVelocity[ID] = RotationBuffer[ID] * glm::inverse(RotationBuffer_Past[ID]) / deltaTime;
}

void UpdateVelocity(float deltaTime, uint32_t ID, glm::vec3* VelocityBuffer, glm::vec3* PositionBuffer, glm::vec3* PositionBuffer_Past) {
	VelocityBuffer[ID]  = (PositionBuffer[ID] - PositionBuffer_Past[ID]) / deltaTime;
}

/////////// Resolve Collisions ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Calculate the closing velocity. Total energy of a physics object in a frame
glm::vec3 ClosingVelocity(glm::vec3 Velocity, glm::quat AngularVelocity, glm::vec3 rotationArm) {
	return Velocity + glm::cross(AngularVelocity, rotationArm);
}

void ApplyForce(float deltaTime, glm::vec3 force, const uint32_t ID, glm::vec3* VelocityBuffer) {
	
	VelocityBuffer[ID] += force * deltaTime;
}

bool ResolvePositionAfterCollision(const float deltaTime, glm::vec3& contact_point, const glm::vec3 ABDeltaPositionResolution, const glm::vec3 normal, const PhysicsComponent& objA, const uint32_t objA_id, const PhysicsComponent& objB, const uint32_t objB_id, glm::vec3* PositionBuffer, glm::vec3* VelocityBuffer) {

	glm::vec3 delta_velocity = (VelocityBuffer[objB_id] - VelocityBuffer[objA_id]) * deltaTime;


	if (CollisionPoint(deltaTime, delta_velocity, PositionBuffer[objA_id], VelocityBuffer[objA_id], PositionBuffer[objB_id], contact_point))
		return;

	float w_sum = objA.inverse_mass + objB.inverse_mass;

	PositionBuffer[objA_id] +=   ABDeltaPositionResolution * (objA.inverse_mass / w_sum);
	PositionBuffer[objB_id] += - ABDeltaPositionResolution * (objB.inverse_mass / w_sum);

}


bool ResolvePositionAfterCollision_World(const PhysicsRayCollision collision, const PhysicsComponent& objA, const uint32_t objA_id, glm::vec3* PositionBuffer, glm::vec3* VelocityBuffer) {
    PositionBuffer[objA_id] += glm::normalize(VelocityBuffer[objA_id])* collision.distanceToPosition;
}


void ResolveVelocitiesAfterCollision(const float deltaTime, float distanceToPosition, glm::vec3 contact_point, glm::vec3 normal,
	glm::vec3* PositionBuffer, glm::quat* RotationBuffer, glm::vec3* VelocityBuffer, glm::quat* AngularVelocityBuffer, 
	glm::vec3* PositionBuffer_Past, glm::quat* RotationBuffer_Past, glm::vec3* VelocityBuffer_Past, glm::quat* AngularVelocityBuffer_Past,
	const PhysicsComponent& objA, uint32_t objA_id,
	const PhysicsComponent& objB, uint32_t objB_id) {

	/// PRESENT CLOSING VELOCITIES
	glm::vec3 A_rotation_arm = (contact_point - (objA.CentreOfRotation + PositionBuffer[objA_id]));
	glm::vec3 B_rotation_arm = (contact_point - (objB.CentreOfRotation + PositionBuffer[objB_id]));

	glm::vec3 A_closing_velocity = ClosingVelocity(VelocityBuffer[objA_id], AngularVelocityBuffer[objA_id], A_rotation_arm);
	glm::vec3 B_closing_velocity = ClosingVelocity(VelocityBuffer[objB_id], AngularVelocityBuffer[objB_id], B_rotation_arm);
	
	/// PAST CLOSING VELOCITIES
	glm::vec3 A_rotation_arm_past = (contact_point - (objA.CentreOfRotation + PositionBuffer_Past[objA_id]));
	glm::vec3 B_rotation_arm_past = (contact_point - (objB.CentreOfRotation + PositionBuffer_Past[objB_id]));

	glm::vec3 A_closing_velocity_past = ClosingVelocity(VelocityBuffer_Past[objA_id], AngularVelocityBuffer_Past[objA_id], A_rotation_arm_past);
	glm::vec3 B_closing_velocity_past = ClosingVelocity(VelocityBuffer_Past[objB_id], AngularVelocityBuffer_Past[objB_id], B_rotation_arm_past);

	// RELATIVES VELOCITIES FOR BOTH TIMEFRAMES
	glm::vec3 velocity_relative = A_closing_velocity - B_closing_velocity;
	glm::vec3 past_velocity_relative = A_closing_velocity_past - B_closing_velocity_past;

	float Normal_Velocity = glm::dot(velocity_relative, normal);
	float Normal_PreVelocity = glm::dot(past_velocity_relative, normal);

	// TANGENTS
	glm::vec3 A_Tangent = glm::cross(A_rotation_arm, normal);
	glm::vec3 B_Tangent = glm::cross(B_rotation_arm, normal);

	// AUGMENTS
	float A_Augment = glm::dot(A_Tangent, A_Tangent) * objA.MomentOfInertia;
	float B_Augment = glm::dot(B_Tangent, B_Tangent) * objB.MomentOfInertia;

	float Augmentation = A_Augment + B_Augment;

	// SOLVE VARIABLES
	float restitution = 0;
	float w_sum = objA.inverse_mass + objB.inverse_mass;
	float restitution_vel = -restitution * Normal_PreVelocity;
	float j = (-Normal_Velocity + restitution_vel) / (w_sum + Augmentation);

	// RESOLVE
	glm::vec3 Velocity_Impulse = normal * j;

	// RESOLVE VELOCITY
	VelocityBuffer[objA_id] += Velocity_Impulse * objA.inverse_mass;
	VelocityBuffer[objB_id] -= Velocity_Impulse * objB.inverse_mass;
	
	// RESOLVE ANGULAR VELOCITY
	AngularVelocityBuffer[objA_id] *=  objA.InverseInertia * glm::cross(Velocity_Impulse, A_rotation_arm);
	AngularVelocityBuffer[objB_id] *= -objB.InverseInertia * glm::cross(Velocity_Impulse, B_rotation_arm);

	/// FRICTION
	
	// TANGENTS
	glm::vec3 Tangent = velocity_relative - Normal_Velocity * normal;
	glm::vec3 A_Frictional_Tangent = glm::cross(A_rotation_arm, Tangent);
	glm::vec3 B_Frictional_Tangent = glm::cross(B_rotation_arm, Tangent);

	// AUGMENTS
	float A_Augment_T = glm::dot(A_Frictional_Tangent, A_Frictional_Tangent) * objA.MomentOfInertia;
	float B_Augment_T = glm::dot(B_Frictional_Tangent, B_Frictional_Tangent) * objB.MomentOfInertia;
	float Augment_T = A_Augment_T + B_Augment_T;

	// CALCULATE FRICTION
	float frictionCoefficient = glm::dot(velocity_relative, A_Tangent); // Could be wrong
	float jt = (-frictionCoefficient) / (w_sum + Augment_T);

	float friction = (objA.StaticFriction + objB.StaticFriction)*.5;
	if(jt > j * friction)
		friction = (objA.DynamicFriction + objB.DynamicFriction)*.5;

	glm::vec3 FrictionImpulse = friction * jt * Tangent;

	// RESOLVE FRICTIONAL VELOCITY
	VelocityBuffer[objA_id] += FrictionImpulse * objA.inverse_mass;
	VelocityBuffer[objB_id] -= FrictionImpulse * objB.inverse_mass;

	// RESOLVE FRICTIONAL ANGULAR VELOCITY
	AngularVelocityBuffer[objA_id] *= objA.InverseInertia * glm::cross(FrictionImpulse, A_rotation_arm);
	AngularVelocityBuffer[objB_id] *= -objB.InverseInertia * glm::cross(FrictionImpulse, B_rotation_arm);
	

}


void ResolveVelocitiesAfterCollision_World(const float deltaTime, float distanceToPosition, glm::vec3 contact_point, glm::vec3 normal,
    glm::vec3* PositionBuffer, glm::quat* RotationBuffer, glm::vec3* VelocityBuffer, glm::quat* AngularVelocityBuffer,
    glm::vec3* PositionBuffer_Past, glm::quat* RotationBuffer_Past, glm::vec3* VelocityBuffer_Past, glm::quat* AngularVelocityBuffer_Past,
    const PhysicsComponent& objA, uint32_t objA_id, glm::vec3 objA_position_collisionPoint) {

    /// PRESENT CLOSING VELOCITIES
    glm::vec3 A_rotation_arm = (contact_point - (objA.CentreOfRotation + PositionBuffer[objA_id]));
    glm::vec3 A_closing_velocity = ClosingVelocity(VelocityBuffer[objA_id], AngularVelocityBuffer[objA_id], A_rotation_arm);

    /// PAST CLOSING VELOCITIES
    glm::vec3 A_rotation_arm_past = (contact_point - (objA.CentreOfRotation + PositionBuffer_Past[objA_id]));
    glm::vec3 A_closing_velocity_past = ClosingVelocity(VelocityBuffer_Past[objA_id], AngularVelocityBuffer_Past[objA_id], A_rotation_arm_past);

    // RELATIVES VELOCITIES FOR BOTH TIMEFRAMES
    glm::vec3 velocity_relative = A_closing_velocity;
    glm::vec3 past_velocity_relative = A_closing_velocity_past;

    float Normal_Velocity = glm::dot(velocity_relative, normal);
    float Normal_PreVelocity = glm::dot(past_velocity_relative, normal);

    // TANGENTS
    glm::vec3 A_Tangent = glm::cross(A_rotation_arm, normal);

    // AUGMENTS
    float Augmentation = glm::dot(A_Tangent, A_Tangent) * objA.MomentOfInertia;

    // SOLVE VARIABLES
    float restitution = 0;
    float w_sum = objA.inverse_mass;
    float restitution_vel = -restitution * Normal_PreVelocity;
    float j = (-Normal_Velocity + restitution_vel) / (w_sum + Augmentation);

    // RESOLVE
    glm::vec3 Velocity_Impulse = normal * j;

    // RESOLVE VELOCITY
    VelocityBuffer[objA_id] += Velocity_Impulse * objA.inverse_mass;

    // RESOLVE ANGULAR VELOCITY
    AngularVelocityBuffer[objA_id] *= objA.InverseInertia * glm::cross(Velocity_Impulse, A_rotation_arm);

    /// FRICTION

    // TANGENTS
    glm::vec3 Tangent = velocity_relative - Normal_Velocity * normal;
    glm::vec3 A_Frictional_Tangent = glm::cross(A_rotation_arm, Tangent);

    // AUGMENTS
    float Augment_T = glm::dot(A_Frictional_Tangent, A_Frictional_Tangent) * objA.MomentOfInertia;

    // CALCULATE FRICTION
    float frictionCoefficient = glm::dot(velocity_relative, A_Tangent); // Could be wrong
    float jt = (-frictionCoefficient) / (w_sum + Augment_T);

    float friction = (objA.StaticFriction + WorldStaticFriction) * .5;
    if (jt > j * friction)
        friction = (objA.DynamicFriction + WorldDynamicFriction) * .5;

    glm::vec3 FrictionImpulse = friction * jt * Tangent;

    // RESOLVE FRICTIONAL VELOCITY
    VelocityBuffer[objA_id] += FrictionImpulse * objA.inverse_mass;

    // RESOLVE FRICTIONAL ANGULAR VELOCITY
    AngularVelocityBuffer[objA_id] *= objA.InverseInertia * glm::cross(FrictionImpulse, A_rotation_arm);
}

//float frictionFraction = glm::dot(delta_velocity, normal);
//
//ApplyForce(frictionFraction, delta_velocity, objA_id, VelocityBuffer);
//ApplyForce(frictionFraction, -delta_velocity, objB_id, VelocityBuffer);