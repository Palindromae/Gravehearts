#pragma once
#include <unordered_map>
#include "Entity.h"
#include "Physics_Const.h"
#include "glm/gtx/hash.hpp"

struct PhysicsPartition {
	std::unordered_map<glm::ivec3, std::unordered_map<int,bool>> EntityMap{};

	glm::ivec3 GetIndex(const glm::ivec3 position) {

		return glm::ivec3(position) & (PhysicsChunkSize - glm::ivec3(1));
	}

	void RemoveEntity(const int id, const glm::ivec3 LastFramePosition_InPhysicsChunkSpace) {
		glm::ivec3 index = GetIndex(LastFramePosition_InPhysicsChunkSpace);
		EntityMap[index].erase(id);

		if (EntityMap[index].size() == 0)
			EntityMap.erase(index);
	}



	void AddEntity(const int id, const glm::ivec3 CurrentFramePosition)
	{
		EntityMap[GetIndex(CurrentFramePosition)][id] = true;
	}

	void AttemptSwap(const int id, const glm::ivec3 PastPosition_InPhysicsChunkSpace, const glm::ivec3 CurrentPosition_InPhysicsChunkSpace) {
		glm::ivec3 PIndex = GetIndex(PastPosition_InPhysicsChunkSpace);
		glm::ivec3 CIndex = GetIndex(CurrentPosition_InPhysicsChunkSpace);

		if (PIndex == CIndex)
			return;

		// Swap
		EntityMap[PIndex].erase(id);
		EntityMap[CIndex][id] = true;
	}

};