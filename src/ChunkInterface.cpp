#include "ChunkInterface.h"

ChunkVolume* ChunkInterface::CreateChunk(ChunkID position) {
	return chunkManager->CreateChunk(position);
}

void ChunkInterface::DeleteChunk(ChunkID position) {
	chunkManager->DeleteChunk(position);
}

MonoidList* ChunkInterface::BindChunkHeader(MonoidList& list) {
	return list.bind(chunkManager->ChunkVolumes);
}

MonoidList* ChunkInterface::BindChunkMemory(MonoidList& list) {
	return list.bind(ChunkGPUMemory)->bind(ChunkGPUMemoryMutexes);
}

MonoidList* ChunkInterface::BindChunkHeader(MonoidList* list) {
	return list->bind(chunkManager->ChunkVolumes);
}

MonoidList* ChunkInterface::BindChunkMemory(MonoidList* list) {
	return list->bind(ChunkGPUMemory)->bind(ChunkGPUMemoryMutexes);
}

void ChunkInterface::TestGeneration(ChunkVolume*& volume, glm::vec3 position)
{
	uint32_t index = GetChunkVolumeIndexFromPos(volume->position, position);
	WorldGeneration->BuildChunkTest(index,position);
}
