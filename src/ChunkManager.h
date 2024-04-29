#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "ChunkVolume.h"
#include "ComputeBuffer.h"
#include <queue>
#include "StoredComputeShaders.h"
#include "MonoidList.h"

class ChunkManager {

	ComputeBuffer* ChunkVolumes{};

	int CurrentPtrMax = 0;
	std::queue<int> reusePTR{};
	inline static int maxVolumes = 25000; //3125000

	std::unordered_map<ChunkID, ChunkVolume*> ChunkVolume_Map{};

	nve::ProductionPackage* context{};

	int GetChunkVolumeID();

	void returnChunkVolumeID(int ID);
public:

	ChunkManager(nve::ProductionPackage* context);

	
	Chunk* CreateChunk(ChunkID position);

	void DeleteChunk(ChunkID position);
};