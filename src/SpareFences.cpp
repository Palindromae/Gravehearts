#include "stdafx.h"
#include "SpareFences.h"
#include "DisjointCommandDispatcher.h"
VkFence* SpareFence::get()
{
	VkFence* fence;
	fences->Pull(fence);
	 DisjointCommandDispatcher::dispatcher->resetFence(*fence);
	 return fence;
	
}
void SpareFence::give(VkFence* fence)
{
	fences->Push(fence);

}
void SpareFence::create(VkFence*& fence) {
	fence = new VkFence{ };
	DisjointCommandDispatcher::dispatcher->createFence(fence);
}

SpareFence::SpareFence(int size) {
	Build(size);
}

void SpareFence::Build(int size) {
	fences = new nve_staticQueue<VkFence*>(size);
	for (size_t i = 0; i < size; i++)
	{
		VkFence* fence = new VkFence{};
		DisjointCommandDispatcher::dispatcher->createFence(fence);
		fences->Push(fence);
	}
}

SpareFence::~SpareFence()
{
	clear();
}


void SpareFence::Rebuild(int size) {
	if (size == -1) {
		size = fences->size;
	}
	clear();
	Build(size);
}

void SpareFence::clear() {
	for (int i = fences->size - 1; i >= 0; i--)
	{

		VkFence* fence = get();

		if (fence != nullptr)
			DisjointCommandDispatcher::dispatcher->destroyFence(*fence);


	}
}