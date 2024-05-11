#pragma once
#include <glm/glm.hpp>

#define ChunkID glm::ivec3
#define ZeroedChunk  0xFF
struct Chunk {
	short quality;

	Chunk(short quality) : quality(quality) {

	}

	void VoidChunk() {
		quality = 0xFF;
	}
};