#pragma once
#include "vulkan/vulkan_core.h"
struct DescriptiveObject {
	VkDescriptorSetLayout layout{};
	VkDescriptorPool pool{};
	VkDescriptorSet set{};

	uint32_t variableCount = 1;

	static const int CommonStages = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR  | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR  |VK_SHADER_STAGE_MISS_BIT_KHR  | VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
};            