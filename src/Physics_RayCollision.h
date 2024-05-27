#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif


const uint NoCollision = 0XFFFF;
const uint WorldCollision = 0XFFFE;
struct PhysicsRayCollision {
	vec3 objA_position;
	int ObjBID; //ID of ObjA is the index
	vec3 objB_position;
	float distanceToPosition;
};

struct PhysicsRayCollision_Work {
	vec3 position;
	int IDOfHit; //
	float distanceToPosition;
	int NoRays;
};