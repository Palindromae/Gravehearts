#pragma once
#include "vulkan/vulkan_core.h"
#include "QueueTypes.h"
namespace nve {


	struct ProductionPackage {
		VkFence fence{};
		VkCommandPool* commandpool;
		VkCommandBuffer* primaryCommandBuffer;
		QueueType type;
		VkCommandBuffer CreateSecondaryCommandBuffer() const;

		ProductionPackage(QueueType type);
		~ProductionPackage();
	};
}