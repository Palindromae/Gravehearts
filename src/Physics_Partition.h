#pragma once
#include <unordered_map>
#include "Entity.h"
#include "Physics_Const.h"
#include "glm/gtx/hash.hpp"

struct PhysicsPartition {
	std::unordered_map<glm::ivec3, std::unordered_map<int,Entity*>> EntityMap{};

	glm::ivec3 GetIndex(glm::vec3 position) {

		return glm::ivec3(position) & PhysicsChunkSize;
	}

	void RemoveEntity(Entity* entity, glm::vec3 LastFramePosition) {
		glm::ivec3 index = GetIndex(LastFramePosition);
		EntityMap[index].erase(entity->ID_Get());

		if (EntityMap[index].size() == 0)
			EntityMap.erase(index);
	}



	void AddEntity(Entity* entity, glm::vec3 CurrentFramePosition) {

		EntityMap[GetIndex(CurrentFramePosition)][entity->ID_Get()] = entity;
	}

};