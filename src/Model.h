#pragma once
#include <glm/glm.hpp>
#include "../AABBDef.h"
struct Model {
	AABBDef aabbdef;
	int DataPtr;
	int Custom;


	Model(glm::vec3 Dimension, int custom =0) : Custom(custom) {
		DataPtr = -1;
		aabbdef.minimum = glm::vec3(0);
		aabbdef.maximum = Dimension;
	}
};