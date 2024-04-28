#include "ChunkInterface.h"

inline Chunk* ChunkInterface::CreateChunk(ChunkID position) {
	return chunkManager->CreateChunk(position);
}

inline void ChunkInterface::DeleteChunk(ChunkID position) {
	chunkManager->DeleteChunk(position);
}
