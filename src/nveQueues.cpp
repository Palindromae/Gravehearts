#include "nveQueues.h"
#include <exception>

void NveQueues::dispatch(QueueType type, const VkSubmitInfo& info, const VkFence& fence)
{
	locks[int(type)]->lock();

	if (vkQueueSubmit(*queues[int(type)], 1, &info, fence) != VK_SUCCESS) {
		throw std::exception("failed to submit");
	}

	locks[int(type)]->unlock();
}

VkResult NveQueues::accessQueue(QueueType type, QueueWork work)
{
	locks[int(type)]->lock();
	VkResult result = work(*queues[int(type)]);
	locks[int(type)]->unlock();
	return result;
}

VkQueue* NveQueues::startQueue(QueueType type)
{
	locks[int(type)]->lock();
	return queues[int(type)];
}

void NveQueues::endQueue(QueueType type)
{
	locks[int(type)]->unlock();

}

NveQueues::NveQueues(std::vector<VkQueue*> queues)
{
	this->queues = queues;
	locks.reserve(queues.size());

	locks.push_back(new std::mutex{});

	for (size_t i = 1; i < queues.size(); i++)
	{
		bool make = true;

		for (size_t j = 0; j < i; j++)
		{
			if (queues[j] == queues[i])
			{
				locks.push_back(locks[j]);
				make = false;
				break;
			}
		}

		if (make) {
			locks.push_back(new std::mutex{});
		}
	}
}
