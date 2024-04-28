#pragma once
#include "ChunkManager.h"

class ChunkInterface {
	static inline ChunkInterface* instance;
	ChunkManager* chunkManager;
	nve::ProductionPackage* context;
public:
	ChunkInterface() {
		if (instance != nullptr)
			throw new std::runtime_error("cannot have more than one chunk manager");

		context = new nve::ProductionPackage(QueueType::Compute);

		chunkManager = new ChunkManager(context);
	}

	~ChunkInterface() {
		delete chunkManager;
		delete context;
	}
	Chunk* CreateChunk(ChunkID position);
	void DeleteChunk(ChunkID position);
};