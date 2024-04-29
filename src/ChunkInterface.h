#pragma once
#include "ChunkManager.h"

class ChunkInterface {
	ChunkManager* chunkManager;
	nve::ProductionPackage* context;
public:
	static inline ChunkInterface* instance;
	ChunkInterface() {
		if (instance != nullptr)
			throw new std::runtime_error("cannot have more than one chunk manager");

		instance = this;

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