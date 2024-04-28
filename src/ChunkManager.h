#pragma once
#define GLM_ENABLE_EXPERIMENTAL
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
public:

	ChunkManager(nve::ProductionPackage* context) : context(context) {
		ChunkVolumes = new ComputeBuffer(ComputeBufferInfo(sizeof(ChunkVolumeGPU), maxVolumes));
	}

	
	Chunk* CreateChunk(ChunkID position) {

		ChunkID VolumePosition = ChunkVolume::ChunkToVolumeID(position);

		if (ChunkVolume_Map[VolumePosition] != nullptr)
		{
			return ChunkVolume_Map[VolumePosition]->GetChunkToBeInitiated(position);
		}

		// Initiate Volume
		ChunkVolume* volume = new ChunkVolume(position, GetChunkVolumeID());
		ChunkVolume_Map[VolumePosition] = volume;


		glm::ivec4 memWrite(position, volume->StoragePtr);

		Shaders::Shaders[Shaders::WriteNewChunkVolume]->dispatch(context, 1, 1, 1, sizeof(glm::ivec4), &memWrite, MonoidList(1).bind(ChunkVolumes)->render());

		Chunk* chunk = volume->GetChunkToBeInitiated(position);

		return chunk;
	}

	void DeleteChunk(ChunkID position) {
		ChunkID volumePosition= ChunkVolume::ChunkToVolumeID(position);

		ChunkVolume* volume = ChunkVolume_Map[position];
		volume->DeleteChunk(position);
		



		if (!volume->ValidateIsVolumeEmpty())
			return;
		
		// Volume is empty so delete
		delete volume;
		ChunkVolume_Map.erase(position);
	}

	

	int GetChunkVolumeID() {
		if (reusePTR.size() > 0){
			int ptr = reusePTR.front();
			reusePTR.pop();

			return ptr;
		}

		if (CurrentPtrMax >= maxVolumes)
			throw std::exception("cant have move than the maximum number of chunks");

		return CurrentPtrMax++;
	}

	int returnChunkVolumeID(int ID) {
		reusePTR.push(ID);
	}
};