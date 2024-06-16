#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Physics_Const.h"
#include "EntityFrameData.h"
#include "../shaders/EntityConst.h"
struct InterpolatedFrameData {

	// Position
	glm::vec3* PositionBuffer;

	// Rotation
	glm::quat* RotationBuffer;

	InterpolatedFrameData() {
		PositionBuffer = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		RotationBuffer = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
	}

	void Copy(EntityFrameData frame) {
		memcpy(PositionBuffer, frame.PositionBuffer, sizeof(glm::vec3) * MaxEntities);
		memcpy(RotationBuffer, frame.VelocityBuffer, sizeof(glm::quat) * MaxEntities);
	}

	void Interpolate(const EntityFrameData* Previous,const EntityFrameData* Next, const float T) {
		for (size_t i = 0; i < MaxEntities; i++)
		{
			PositionBuffer[i] = glm::mix(Previous->PositionBuffer[i], Next->PositionBuffer[i], glm::vec3(T));
			RotationBuffer[i] = glm::slerp(Previous->RotationBuffer[i], Next->RotationBuffer[i], T);
		}
	}

	//// OVERRIDED forcing copying rather than setting equality
	//EntityFrameData& operator=(const EntityFrameData& a)
	//{
	//	Copy(a);
	//	return *this;  // Return a reference to myself.
	//}
};