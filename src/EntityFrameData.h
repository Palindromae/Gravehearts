#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Physics_Const.h"
#include "../shaders/EntityConst.h"
struct EntityFrameData {
	
	// Position
	glm::vec3* PositionBuffer;
	glm::vec3* VelocityBuffer;

	// Rotation
	glm::quat* RotationBuffer;
	glm::quat* AngularVelocityBuffer;

	EntityFrameData() {
		PositionBuffer = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		VelocityBuffer = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);

		RotationBuffer = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		AngularVelocityBuffer = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
	}

	void Copy(EntityFrameData frame) {
		memcpy(PositionBuffer, frame.PositionBuffer, sizeof(glm::vec3) * MaxEntities);
		memcpy(VelocityBuffer, frame.VelocityBuffer, sizeof(glm::vec3) * MaxEntities);

		memcpy(RotationBuffer, frame.VelocityBuffer, sizeof(glm::quat) * MaxEntities);
		memcpy(AngularVelocityBuffer, frame.AngularVelocityBuffer, sizeof(glm::quat) * MaxEntities);
	}
	
	//// OVERRIDED forcing copying rather than setting equality
	//EntityFrameData& operator=(const EntityFrameData& a)
	//{
	//	Copy(a);
	//	return *this;  // Return a reference to myself.
	//}
};