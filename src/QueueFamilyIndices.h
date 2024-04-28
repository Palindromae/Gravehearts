#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <optional>
#include <vector>

struct QueueFamilyIndices {
	std::optional<uint32_t > graphicsFamily;
	std::optional<uint32_t > presentFamily;
	std::optional<uint32_t > computeFamily;

	bool isComplete();
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const VkSurfaceKHR& surface);