#include "ChunkManager.h"

ChunkManager::ChunkManager(nve::ProductionPackage* context) : context(context) {
	ChunkVolumes = new ComputeBuffer(ComputeBufferInfo(sizeof(ChunkVolumeGPU), maxVolumes));
}

Chunk* ChunkManager::CreateChunk(ChunkID position) {

	ChunkID VolumePosition = ChunkVolume::ChunkToVolumeID(position);
	ChunkVolume* volume;
	if (ChunkVolume_Map[VolumePosition] != nullptr)
	{
		volume = ChunkVolume_Map[VolumePosition];
	}
	else {
		// Initiate Volume
		volume = new ChunkVolume(position, GetChunkVolumeID());
		ChunkVolume_Map[VolumePosition] = volume;

		glm::ivec4 memWrite(position, volume->StoragePtr);
		Shaders::Shaders[Shaders::WriteNewChunkVolume]->dispatch(context, 1, 1, 1, sizeof(glm::ivec4), &memWrite, MonoidList(1).bind(ChunkVolumes)->render());
	}




	Chunk* chunk = volume->GetChunkToBeInitiated(position);

	return chunk;
}

void ChunkManager::DeleteChunk(ChunkID position) {
	ChunkID volumePosition = ChunkVolume::ChunkToVolumeID(position);

	ChunkVolume* volume = ChunkVolume_Map[position];
	volume->DeleteChunk(position);




	if (!volume->ValidateIsVolumeEmpty())
		return;

	returnChunkVolumeID(volume->StoragePtr);
	// Volume is empty so delete
	delete volume;
	ChunkVolume_Map.erase(position);
}

int ChunkManager::GetChunkVolumeID() {
	if (reusePTR.size() > 0) {
		int ptr = reusePTR.front();
		reusePTR.pop();

		return ptr;
	}

	if (CurrentPtrMax >= maxVolumes)
		throw std::exception("cant have move than the maximum number of chunks");

	return CurrentPtrMax++;
}

void ChunkManager::returnChunkVolumeID(int ID) {
	reusePTR.push(ID);
}
