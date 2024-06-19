#pragma once
#include <glm/glm.hpp>

struct TranslationCommand {
	bool Delta;
	glm::vec3 TranslationVector;

	void Reset() {
		Delta = true;
		TranslationVector = {};
	}
};