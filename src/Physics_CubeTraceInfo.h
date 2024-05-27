
#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif

struct PhysicsCubeTraceInfo {
	vec3 position;
	int ID;
	vec3 direction;
	int quality;
	float MaxDistance;
};