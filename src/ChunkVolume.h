#pragma once
#include "Chunk.h"
#include "../shaders/ChunkGPUConst.h"

constexpr float NoChunksPerAxii_inv = 1 / (float)NoChunksPerAxii;

// Defines a region of chunks
struct ChunkVolume {
	int StoragePtr;
	Chunk* chunks;
	ChunkID position;

	uint64_t* masks{};
	
	ChunkVolume(ChunkID pos, int storageID) : position(pos), StoragePtr(storageID) {
		//Get storage ptr

		// Create Chunks
		chunks = (Chunk*) malloc(sizeof(Chunk) * NoChunksPerVolume);

		for (size_t x = 0; x < NoChunksPerAxii; x++)
		{
			for (size_t y = 0; y < NoChunksPerAxii; y++)
			{
				for (size_t z = 0; z < NoChunksPerAxii; z++)
				{

					chunks[ChunkHeaderIndex(glm::ivec3(x, y, z))] = Chunk(ZeroedChunk);
				}
			}
		}

		masks = new uint64_t[8];

		masks[0] = 0;
		masks[1] = 0;
		masks[2] = 0;
		masks[3] = 0;
		masks[4] = 0;
		masks[5] = 0;
		masks[6] = 0;
		masks[7] = 0;
	}


	static ChunkID ChunkToVolumeID(ChunkID ID) {

		ID = ID >> BitsPerAxii;
		return ID;
	}

	Chunk* GetChunk(ChunkID pos) {
		return &chunks[GetChunkVolumeIndexFromPos(position, pos)];
	}
	// Call this if the chunk is being initated
	Chunk* GetChunkToBeInitiated(ChunkID pos) {

		uint32_t id = GetChunkVolumeIndexFromPos(position, pos);

		Chunk* chunk = &chunks[id];
		int maskID = id / 64;
		masks[maskID] |= uint64_t(1) << (id % 64);
		return chunk;
	}



	void DeleteChunk(ChunkID ID) {
		chunks[GetChunkVolumeIndexFromPos(position, ID)].VoidChunk();
	}

	bool ValidateIsVolumeEmpty() {

		uint64_t processedMask = masks[0] | masks[1] | masks[2] | masks[3] | masks[4] | masks[5] | masks[7];

		return processedMask == 0;
	}

private:

};

struct ChunkVolumeGPU {
	glm::ivec3 pos;
private:
	int dataPtr[NoChunksPerVolume]; // Not CPU set
};