#pragma once
#include <unordered_map>
#include "Entity.h"
struct PhysicsPartition {
	std::unordered_map<glm::ivec3, std::vector<Entity*>> EntityMap{};

	glm::ivec3 GetIndex(glm::vec3 position) {

		return glm::ivec3(position) & PhysicsChunkSize;
	}

	void RemoveEntity(Entity* entity) {
		EntityMap.erase(id);
	}

	void AddEntity(Entity* entity) {
		EntityMap[entity->ID_Get()] = entity;
	}

};