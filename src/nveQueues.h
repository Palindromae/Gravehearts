#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <mutex>
#include "QueueTypes.h"
#include <functional>
#include "NveTypes.h"

struct NveQueues {

	std::vector<VkQueue*> queues{};
	std::vector<std::mutex*> locks{};
	void dispatch(QueueType type, const VkSubmitInfo& info, const VkFence& fence);
	VkResult accessQueue(QueueType type, QueueWork work);

	VkQueue* startQueue(QueueType type);
	void endQueue(QueueType type);

	NveQueues(std::vector<VkQueue*> queues);

};