#include "stdafx.h"
#include "SingleTimeCommands.h"
#include  <exception>
#include "QueueTypes.h"

VkCommandBuffer invokeSingleTimeCommands(const VkDevice& lDevice, const VkCommandPool& commandPool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(lDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void FinishSingleTimeCommands(VkCommandBuffer commandBuffer, const VkDevice& device, const VkCommandPool& commandPool, const VkQueue* graphicsQueue, VkFence* fence) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	if (fence != nullptr) {

		if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, *fence) != VK_SUCCESS)
			throw std::exception("failed to submit queue");

		vkWaitForFences(device, 1, fence, true, UINT64_MAX);
		vkResetFences(device, 1, fence);
	}
	else {
		if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			throw std::exception("failed to submit queue");
		vkQueueWaitIdle(*graphicsQueue);
	}

;

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void SingleTimeCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const VkDevice& device, const VkCommandPool& commandPool,  const VkQueue* graphicsQueue, const VkDeviceSize srcOffset, const VkDeviceSize dstOffset, VkFence* fence) {
	VkCommandBuffer commandBuffer = invokeSingleTimeCommands(device, commandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	copyRegion.dstOffset = dstOffset;
	copyRegion.srcOffset = srcOffset;
	
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	FinishSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue, fence);
}

