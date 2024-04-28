#pragma once
#include <vulkan/vulkan_core.h>
#include "nve_staticQueue.h"

class SpareFence {

	nve_staticQueue<VkFence*>* fences;

public: 
	VkFence* get();
	void give(VkFence* fence);
	void create(VkFence*& fence);
	SpareFence(int size);
	void Build(int size);
	~SpareFence();
	void Rebuild(int size = -1);
	void clear();
};