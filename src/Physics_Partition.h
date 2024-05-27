#pragma once
#include <unordered_map>
#include "Entity.h"
struct PhysicsPartition {
	std::unordered_map<int, Entity*> EntityMap{};

	void RemoveEntity(int id) {
		EntityMap.erase(id);
	}

	void AddEntity(Entity* entity) {
		EntityMap[entity->ID_Get()] = entity;
	}
};