#pragma once
#include <vulkan/vulkan_core.h>

VkCommandBuffer invokeSingleTimeCommands(const VkDevice& lDevice, const VkCommandPool& commandPool);
void FinishSingleTimeCommands(VkCommandBuffer commandBuffer, const VkDevice& device, const VkCommandPool& commandPool, const VkQueue* graphicsQueue, VkFence* fence = VK_NULL_HANDLE);
void SingleTimeCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const VkDevice& device, const VkCommandPool& commandPool, const VkQueue* graphicsQueue, const VkDeviceSize srcOffset = 0, const VkDeviceSize dstOffset = 0, VkFence* fence = VK_NULL_HANDLE);
