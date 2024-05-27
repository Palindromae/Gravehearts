#pragma once
#include "vulkan/vulkan_core.h"
#include <corecrt_malloc.h>
#include <cassert>
#include "DisjointCommandDispatcher.h"
class CallbackGPUMemory {

	VkDeviceAddress length{};
	VkDeviceMemory* referencedMemory{};
	void* data;
public:

	inline static VkMemoryPropertyFlags IdealPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

	/// <summary>
	// 
	/// 
	/// </summary>
	/// <param name="referencedMemory"> Must be allocated with VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and should be HOST_CACHED can speed up referencing set</param>
	/// <param name="length"></param>
	CallbackGPUMemory(VkDeviceMemory* referencedMemory, VkDeviceAddress length) : referencedMemory(referencedMemory), length(length) {

		// Make sure that we're mapping real memory
		assert(referencedMemory != nullptr);
		assert(length != 0);

		CommandDispatcher->MapMemory(referencedMemory, length, data);
	}

	/// <summary>
	/// You must prevent any writes from occuring before reading this
	/// </summary>
	/// <returns></returns>
	void* GetMemory() {
		return data;
	}
	/// <summary>
	/// You must prevent any writes from occuring before reading this, however this copy can be freely editted and accessed without risk of writes
	/// </summary>
	/// <returns></returns>
	void* GetMemoryCopy() {
		void* memcpy = malloc(length);
		std::memcpy(memcpy,data, length);

		return memcpy;
	}

	~CallbackGPUMemory() {
		CommandDispatcher->UnmapMemory(referencedMemory);
	}
};