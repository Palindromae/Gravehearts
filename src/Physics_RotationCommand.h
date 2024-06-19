#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct RotationCommand {
	bool Delta;
	glm::quat RotationVector;

	void Reset() {
		Delta = true;
		RotationVector = glm::quat();
	}
};