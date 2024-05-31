
#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using vec4 = glm::quat;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif

struct PhysicsCubeTraceInfo {
	vec3 positionA;
	int ModelTypeA;
	vec3 positionB;
	int ModelTypeB;
	vec3 direction;
	float MaxDistance;
	vec4 RotationA;
	vec4 RotationB;
};