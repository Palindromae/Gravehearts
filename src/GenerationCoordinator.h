#pragma once
#include "ComputeBuffer.h"
#include "../shaders/ChunkGPUConst.h"
#include "ChunkConst.h"
#include "Chunk.h"

class GenerationCoordinator {
	ComputeBuffer* UnexpandedNodeA;
	ComputeBuffer* UnexpandedNodeB;
	ComputeBuffer* IndirectBuffer;
    nve::ProductionPackage* context{};
public:

    GenerationCoordinator();

    void BuildChunkTest(uint32_t chunkID, ChunkID chunk_pos);
};

/*

layout(set = 0, binding = 0)  buffer  ChunkHeadersN{
    ChunkHeader[] ChunkHeaders;
};

layout(set = 1, binding = 0)  buffer  GPUMemoryN{
    VoxelBrick[] GPUMemory;
};

layout(set = 2, binding = 0)  buffer  fromN{
    UnexpandedNode[] fromNodes;
};

layout(set = 3, binding = 0)  buffer  toN{
    UnexpandedNode[] toNodes;
};


layout(set = 4, binding = 0)  buffer  GPUVoxelMutexesN{
    int[] GPUVoxelMutexes;
};


*/