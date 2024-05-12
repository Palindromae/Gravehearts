#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "ChunkVolume.h"
#include "ComputeBuffer.h"
#include <queue>
#include "StoredComputeShaders.h"
#include "MonoidList.h"
#include "Tlas.h"
#include "ShaderPushConstants.h"

class ChunkManager {


	int CurrentPtrMax = 0;
	std::queue<int> reusePTR{};
	inline static int maxVolumes = ceil(3125000.0 / NoChunksPerVolume);

	std::unordered_map<ChunkID, ChunkVolume*> ChunkVolume_Map{};

	nve::ProductionPackage* context{};

	int GetChunkVolumeID();

	void returnChunkVolumeID(int ID);
	void CreateChunkBlas();

	Tlas ChunkTlas{};
	nvvk::Buffer               BlasBuffer;
	ComputeBuffer*             instanceBuffer{};
	VkAccelerationStructureKHR ChunkBlas{};

	int MajorChange;

	CreateChunkConst createConsts{};
	CreateChunkConst nullConsts{};
public:
	ComputeBuffer* ChunkVolumes{};

	ChunkManager(nve::ProductionPackage* context);

	
	ChunkVolume* CreateChunk(ChunkID position);

	void DeleteChunk(ChunkID position);

	void UpdateGPUStructure();

	void checkChunkArr() {
		int amount = ChunkVolume_Map.size();

		ChunkVolumeGPU* data = (ChunkVolumeGPU*)malloc(sizeof(ChunkVolumeGPU) * amount);

		ChunkVolumes->readBufferData(data,0, sizeof(ChunkVolumeGPU) * amount, context);

		int a = 0;
	}
};