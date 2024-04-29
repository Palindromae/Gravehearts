#include "ChunkInterface.h"

Chunk* ChunkInterface::CreateChunk(ChunkID position) {
	return chunkManager->CreateChunk(position);
}

void ChunkInterface::DeleteChunk(ChunkID position) {
	chunkManager->DeleteChunk(position);
}
