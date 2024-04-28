#pragma once
#include "Chunk.h"

constexpr int NoChunksPerAxii = 5; //set ChunkVolumeGPU and chunk Header before changing and no masks and No Chunks Volumes Total
constexpr float NoChunksPerAxii_inv = 1/(float)NoChunksPerAxii;
constexpr int NoChunksPerVolume = NoChunksPerAxii * NoChunksPerAxii * NoChunksPerAxii;
constexpr int BitsPerAxii = 3;

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

					chunks[index(x, y, z)] = Chunk(ZeroedChunk);
				}
			}
		}

		masks = new uint64_t[2];

		masks[0] = 0;
		masks[1] = 0;
	}


	static ChunkID ChunkToVolumeID(ChunkID ID) {

		ID *= NoChunksPerAxii_inv;
		return ID;
	}

	Chunk* GetChunk(ChunkID pos) {
		ChunkID rel_pos = pos - position;
		return &chunks[index(rel_pos.x, rel_pos.y, rel_pos.z)];
	}
	// Call this if the chunk is being initated
	Chunk* GetChunkToBeInitiated(ChunkID pos) {
		ChunkID rel_pos = pos - position;

		uint32_t id = index(rel_pos.x, rel_pos.y, rel_pos.z);

		Chunk* chunk = &chunks[id];
		int maskID = id / 64;
		masks[maskID] |= uint64_t(1) << (maskID % 64);
		return chunk;
	}

	int index(int x, int y, int z) {

		int index = x;
		index += y << BitsPerAxii;
		index += z << BitsPerAxii;

		return index;
	}

	void DeleteChunk(ChunkID ID) {
		ID = ID - position;
		chunks[index(ID.x, ID.y, ID.z)].DeleteChunk();
	}

	bool ValidateIsVolumeEmpty() {

		uint64_t processedMask = masks[0] & masks[1];

		return processedMask == UINT64_MAX;
	}

};

struct ChunkVolumeGPU {
	glm::vec3 pos;
private:
	int dataPtr[125]; // Not CPU set
};