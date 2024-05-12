#pragma once
#include "ChunkManager.h"
#include "GenerationCoordinator.h"
#include "../shaders/VoxelBrick.h"

class ChunkInterface {
	ChunkManager* chunkManager;
	nve::ProductionPackage* context;
	GenerationCoordinator* WorldGeneration{};

	ComputeBuffer* ChunkGPUMemory{};
	ComputeBuffer* ChunkGPUMemoryMutexes{};

public:
	static inline ChunkInterface* instance;
	ChunkInterface() {
		if (instance != nullptr)
			throw new std::runtime_error("cannot have more than one chunk manager");

		instance = this;

		context = new nve::ProductionPackage(QueueType::Compute);

		chunkManager = new ChunkManager(context);
		WorldGeneration = new GenerationCoordinator();

		ChunkGPUMemory = new ComputeBuffer(ComputeBufferInfo(sizeof(VoxelBrick),nve::GPUMemory::TotalMemory / sizeof(VoxelBrick)));
		ChunkGPUMemoryMutexes = new ComputeBuffer(ComputeBufferInfo(sizeof(int), nve::GPUMemory::chapters));
		
	}

	~ChunkInterface() {
		delete chunkManager;
		delete context;
	}
	ChunkVolume* CreateChunk(ChunkID position);
	void DeleteChunk(ChunkID position);
	

	MonoidList* BindChunkHeader(MonoidList& list);

	MonoidList* BindChunkMemory(MonoidList& list);

	MonoidList* BindChunkHeader(MonoidList* list);

	MonoidList* BindChunkMemory(MonoidList* list);

	void CheckChunkArr() {
		chunkManager->checkChunkArr();
	}

	void CheckGPUMemory() {
		int length = 1000;

		VoxelBrick* data = (VoxelBrick*)malloc(sizeof(VoxelBrick) * length);

		ChunkGPUMemory->readBufferData(data, sizeof(VoxelBrick) * 0, sizeof(VoxelBrick) * length, context);
		std::vector<VoxelBrick> mem{};
		for (size_t i = 0; i < length; i++)
		{
			mem.push_back(data[i]);
		}
		CheckChunkArr();
		
		int a = 0;
		free(data);
	}

	void RebuildGPUStructures() {
		chunkManager->UpdateGPUStructure();
	}

	void TestGeneration(ChunkVolume*& volume, glm::vec3 position);
};