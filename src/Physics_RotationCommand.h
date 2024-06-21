#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct RotationCommand {
	bool Delta = true;
	glm::quat RotationVector = glm::quat();

	void Reset() {
		Delta = true;
		RotationVector = glm::quat();
	}
};