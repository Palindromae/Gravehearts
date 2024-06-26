#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif

struct AABBDef
{
#ifdef __cplusplus
	vec3 minimum{0, 0, 0};  // Aabb
	vec3 maximum{0, 0, 0};  // Aabb
#else
	// GLSL definition without defaults
	vec3 minimum; 
	vec3 maximum;
#endif
};