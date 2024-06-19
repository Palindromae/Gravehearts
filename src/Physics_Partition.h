#pragma once
#include <unordered_map>
#include "Entity.h"
#include "Physics_Const.h"
#include "glm/gtx/hash.hpp"

struct PhysicsPartition {
	std::unordered_map<glm::ivec3, std::unordered_map<int,bool>> EntityMap{};

	glm::ivec3 GetIndex(glm::vec3 position) {

		return glm::ivec3(position) & PhysicsChunkSize;
	}

	void RemoveEntity(int id, glm::vec3 LastFramePosition) {
		glm::ivec3 index = GetIndex(LastFramePosition);
		EntityMap[index].erase(id);

		if (EntityMap[index].size() == 0)
			EntityMap.erase(index);
	}



	void AddEntity(int id, glm::vec3 CurrentFramePosition) {

		EntityMap[GetIndex(CurrentFramePosition)][id] = true;
	}

};