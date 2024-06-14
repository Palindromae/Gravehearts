#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif

#include "AABBDef.h"
struct Model {
	AABBDef aabbdef;
	int DataPtr;
	int Custom;
#ifdef __cplusplus
	Model(glm::vec3 Dimension, int custom =0) : Custom(custom) {
		DataPtr = -1;
		aabbdef.minimum = glm::vec3(0);
		aabbdef.maximum = Dimension;
	}
#endif
};