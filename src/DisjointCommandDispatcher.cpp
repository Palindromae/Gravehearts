#include "stdafx.h"
#include "DisjointCommandDispatcher.h"
#include <iostream>
#include "trace.h"
#include "FilePath.h"
#include <stb_image.h>
//#include "ComputeBuffer.h"
//#include "ShaderPushConstants.h"
//#include "MonoidList.h"
//#include "StoredComputeShaders.h"
//#include "DisjointGlobalMemory.h"
#include "QueueFamilyIndices.h"
#include "ReadShader.h"
#include "vulkan/vulkan_core.h"

VkCommandBuffer DisjointCommandDispatcher::endSingleTimeCommands(QueueType type, VkCommandBuffer commandBuffer, VkFence* fence, const bool WaitOnFence, const bool ResetFence)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	
	VkFence vkFence{};

	if (fence != nullptr) {
		vkFence = *fence;
	}

	auto submittion = [submitInfo, vkFence](VkQueue queues) {return vkQueueSubmit(queues, 1, &submitInfo, vkFence);  };
	auto result = queues->accessQueue(type, submittion);

	if (result != VK_SUCCESS) {
		throw std::exception("failed to submit");
	}
	/*if (fence != nullptr)
	{
		resetFence(*fence);


		if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, *fence) != VK_SUCCESS) {
			throw std::exception("failed to submit");
		}
	}
	else 
		if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::exception("failed to submit");
	}
	*/
	

	if (WaitOnFence){
		if (fence != nullptr) {
			vkWaitForFences(*device, 1, fence, VK_TRUE, UINT64_MAX);
			//vkQueueWaitIdle(*graphicsQueue);

			if(ResetFence)
				resetFence(*fence);

			vkFreeCommandBuffers(*device, *commandPool, 1, &commandBuffer);
		
			return nullptr;
		}
	}
	

	return commandBuffer;
	
}

void DisjointCommandDispatcher::WaitTilIdle(nve::ProductionPackage* context) {
	auto submittion = [](VkQueue queues) {return vkQueueWaitIdle(queues);  };
	auto result = queues->accessQueue(context->type, submittion);

	if (result != VK_SUCCESS) {
		throw std::exception("failed to submit");
	}
}

VkCommandBuffer DisjointCommandDispatcher::endSingleTimeCommands(nve::ProductionPackage* context, VkCommandBuffer commandBuffer, const bool WaitOnFence, const bool ResetFence)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFence vkFence = context->fence;

	auto submittion = [submitInfo, vkFence](VkQueue queues) {return vkQueueSubmit(queues, 1, &submitInfo, vkFence);  };
	auto result = queues->accessQueue(context->type, submittion);

	if (result != VK_SUCCESS) {
		throw std::exception("failed to submit");
	}
	/*if (fence != nullptr)
	{
		resetFence(*fence);


		if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, *fence) != VK_SUCCESS) {
			throw std::exception("failed to submit");
		}
	}
	else
		if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::exception("failed to submit");
	}
	*/


	if (WaitOnFence) {
		
			vkWaitForFences(*device, 1, &context->fence, VK_TRUE, UINT64_MAX);
			//vkQueueWaitIdle(*graphicsQueue);

			if (ResetFence)
				resetFence(context->fence);

			vkFreeCommandBuffers(*device, *context->commandpool, 1, &commandBuffer);

			return nullptr;
		
	}


	return commandBuffer;
}


void DisjointCommandDispatcher::WaitOnOneFenceMax(const VkFence* fence) {
	vkWaitForFences(*device, 1, fence, VK_TRUE, UINT64_MAX);
}

void DisjointCommandDispatcher::destroyFence(VkFence& fence) {
	vkDestroyFence(*device, fence, nullptr);
}

void DisjointCommandDispatcher::createFence(VkFence* fence, VkFenceCreateFlagBits flag)
{
	VkFenceCreateInfo info{};

	info.flags = flag;
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence(*device, &info, nullptr, fence);
}

void DisjointCommandDispatcher::CreateDescriptorPool(VkDescriptorPoolCreateInfo& poolInfo, VkDescriptorPool& pool) {
	if (vkCreateDescriptorPool(*device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool");
}

void DisjointCommandDispatcher::BindSparseMemory(VkBindSparseInfo& info, const VkFence& fence)
{
	auto queue = queues->startQueue(QueueType::Compute);
	vkQueueBindSparse(*queue, 1, &info, fence);
	auto res = vkWaitForFences(*device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(*device, 1, &fence);
	trace_debug("res=%d", res);
	queues->endQueue(QueueType::Compute);
}

void DisjointCommandDispatcher::GetImageRequirements(const VkImage& image, uint32_t* count, VkSparseImageMemoryRequirements& req)
{
	vkGetImageSparseMemoryRequirements(*device, image, count, &req);
}

void DisjointCommandDispatcher::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& layoutInfo, VkDescriptorSetLayout& layout) {
	if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout");
}

void DisjointCommandDispatcher::createDescriptorSets(const VkDescriptorSetAllocateInfo& allocInfo, VkDescriptorSet& set) {
	auto a = vkAllocateDescriptorSets(*device, &allocInfo, &set);
	if (a != VK_SUCCESS)
		throw std::runtime_error("failed to allocate descriptor sets");
}

void DisjointCommandDispatcher::updateDescriptorSets(int writeCount, VkWriteDescriptorSet& descriptorWrites, uint32_t descriptorCopyCount) {
	vkUpdateDescriptorSets(*device, writeCount, &descriptorWrites, descriptorCopyCount, nullptr);
}
void DisjointCommandDispatcher::updateDescriptorSets(int writeCount, VkWriteDescriptorSet* descriptorWrites, uint32_t descriptorCopyCount) {
	vkUpdateDescriptorSets(*device, writeCount, descriptorWrites, descriptorCopyCount, nullptr);
}

//Buffer Commands
void DisjointCommandDispatcher::createBuffer(const VkBufferCreateInfo& bufferInfo, VkBuffer& buffer) {
	if (vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer");
	}
}
uint32_t DisjointCommandDispatcher::findMemoryType(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (!(memoryRequirements.memoryTypeBits & (1 << i))) {
			continue;
		}
		if ((memProperties.memoryTypes[i].propertyFlags & properties) != properties) {
			continue;
		}
		if (memProperties.memoryHeaps[memProperties.memoryTypes[i].heapIndex].size >= memoryRequirements.size) {
			return i;
		}
	}

	return UINT32_MAX;
}


bool DisjointCommandDispatcher::GenerateMemoryAllocateInfo(const VkMemoryRequirements& req, VkMemoryPropertyFlags& properties, VkDeviceMemory& bufferMemory, bool requires_exact_device_match, VkMemoryAllocateInfo& allocInfo) {

	uint32_t index = findMemoryType(req, properties);
	if (index == UINT32_MAX) {
		if (requires_exact_device_match) {
			trace_error("failed to find suitable memory");
			throw std::runtime_error("failed to find suitable memory");
		}
		index = findMemoryType(req, (properties & ~VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
		if (index != UINT32_MAX) {
			properties &= ~VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}
	}
	if (index == UINT32_MAX) {
		trace_error("failed to find suitable memory");
		return false;
	}

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = req.size;
	allocInfo.memoryTypeIndex = index;

	return true;
}

VkMemoryRequirements DisjointCommandDispatcher::GetBufferMemoryRequirements(const VkBuffer& buffer) {
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(*device, buffer, &memRequirements);
	return memRequirements;
}


bool DisjointCommandDispatcher::allocateBufferMemory(const VkBuffer& buffer, VkMemoryPropertyFlags& properties, const VkMemoryPropertyFlags& memflags, VkDeviceMemory& bufferMemory, bool requires_exact_device_match) {

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(*device, buffer, &memRequirements);

	VkMemoryAllocateFlagsInfo flagInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
	flagInfo.flags = memflags;
	

	VkMemoryAllocateInfo allocInfo{};
	if (!GenerateMemoryAllocateInfo(memRequirements, properties, bufferMemory, requires_exact_device_match, allocInfo))
	{
		trace_error("failed to generate memory allocation info");
		throw std::runtime_error("failed to generate buffer memory info");
	}

	allocInfo.pNext = &flagInfo;

	if (vkAllocateMemory(*device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		trace_error("failed to allocate buffer memory");
		throw std::runtime_error("failed to generate memory allocation info");
	}
	vkBindBufferMemory(*device, buffer, bufferMemory, 0);
	return true;
}

void DisjointCommandDispatcher::setStagingBuffer(VkDeviceMemory& stagingBufferMemory, VkDeviceSize bufferSize, const void* dataArray) {
	void* data;

	vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, dataArray, (size_t)bufferSize);
	vkUnmapMemory(*device, stagingBufferMemory);
}

void DisjointCommandDispatcher::MapMemory(VkDeviceMemory* memory, VkDeviceSize length, void*& data) {
	vkMapMemory(*device, *memory, 0, length, 0, &data);
}
void DisjointCommandDispatcher::UnmapMemory(VkDeviceMemory* memory) {
	vkUnmapMemory(*device, *memory);
}

void DisjointCommandDispatcher::copyDataToBuffer(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, VkDeviceSize offset, const void* data) {
	//this is inefficent premap the memory perma
	void* mapMemory;

	vkMapMemory(*device, bufferMemory, offset, bufferSize, 0, &mapMemory);

	memcpy(mapMemory, data, bufferSize);

	vkUnmapMemory(*device, bufferMemory);
}


void DisjointCommandDispatcher::copyDataToBuffer(VkBuffer& stagingBuffer, VkDeviceMemory stagingBufferMemory, VkBuffer& buffer,VkDeviceSize offset, VkDeviceSize bufferSize, nve::ProductionPackage* context, VkFence* fence) {
	
	auto usedCommandPool = commandPool;  
	auto type = QueueType::Graphics;

	if (context != nullptr) {
		usedCommandPool = context->commandpool;
		type = context->type;
	}

	auto queue = queues->startQueue(type);
	

	SingleTimeCopyBuffer(stagingBuffer, buffer, bufferSize, *device, *usedCommandPool, queue, 0, offset, fence);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);

	queues->endQueue(type);
}



void DisjointCommandDispatcher::copyBufferToBuffer(VkBuffer& src, VkBuffer& dst, uint32_t regions, const VkBufferCopy* copies, nve::ProductionPackage* context) {


	auto usedCommandPool = commandPool;
	auto type = QueueType::Graphics;

	if (context != nullptr) {
		usedCommandPool = context->commandpool;
		type = context->type;
	}



	auto queue = queues->startQueue(type);
	
	
	VkCommandBuffer commandBuffer = invokeSingleTimeCommands(*device, *usedCommandPool);



	vkCmdCopyBuffer(commandBuffer, src, dst, regions, copies);

	VkFence* fence = Fences->get();
	vkCmdCopyBuffer(commandBuffer, src, dst, regions, copies);
	FinishSingleTimeCommands(commandBuffer, *device, *usedCommandPool, queue, fence);
	Fences->give(fence);
	queues->endQueue(type);

}

void DisjointCommandDispatcher::copyDataFromBuffer(VkBuffer& stagingBuffer, VkDeviceMemory stagingBufferMemory, VkBuffer& buffer, VkDeviceSize offset, VkDeviceSize bufferSize, void* dataOut, nve::ProductionPackage* context) {
	
	auto usedCommandPool = commandPool;
	auto type = QueueType::Graphics;

	if (context != nullptr) {
		usedCommandPool = context->commandpool;
		type = context->type;
	}

	void* data;
	//transfer buffer to be destination
	vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
	auto queue = queues->startQueue(type);
	SingleTimeCopyBuffer(buffer, stagingBuffer, bufferSize, *device, *usedCommandPool, queue,offset);
	queues->endQueue(type);
	memcpy(dataOut, data, (size_t)bufferSize);
	vkUnmapMemory(*device, stagingBufferMemory);

	//Destory after use
	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}
void DisjointCommandDispatcher::destroyBuffer(VkBuffer& buffer) {
	vkDestroyBuffer(*device, buffer, nullptr);
}
void DisjointCommandDispatcher::destroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkDescriptorPool& pool, VkDescriptorSetLayout& layout) {
	vkDestroyBuffer(*device, buffer, nullptr);
	vkFreeMemory(*device, memory, nullptr);
	vkDestroyDescriptorPool(*device, pool, nullptr);
	vkDestroyDescriptorSetLayout(*device, layout, nullptr);
}

void DisjointCommandDispatcher::destroyDescriptors(VkDescriptorPool& pool, VkDescriptorSetLayout& layout) {
	vkDestroyDescriptorPool(*device, pool, nullptr);
	vkDestroyDescriptorSetLayout(*device, layout, nullptr);
}
void DisjointCommandDispatcher::destroyLayouts(VkDescriptorSetLayout* layout, int count) {
	for (size_t i = 0; i < count; i++)
	{
	}vkDestroyDescriptorSetLayout(*device, layout[0], nullptr);

}

void DisjointCommandDispatcher::blitImage(VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout, int regionCount, VkImageBlit* regions, VkFilter filter, VkFence* fence, const bool resetFence)
{
	VkCommandBuffer buffer = invokeSingleTimeCommands(*device, *commandPool);

	vkCmdBlitImage(buffer, src, srcLayout, dst, dstLayout, regionCount, regions, filter);

	endSingleTimeCommands(QueueType::Graphics, buffer, fence, true, resetFence);
}

void DisjointCommandDispatcher::copyImage(VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout, int regionCount, const VkImageCopy* regions)
{
	VkCommandBuffer buffer = invokeSingleTimeCommands(*device, *commandPool);

	vkCmdCopyImage(buffer, src, srcLayout, dst, dstLayout, regionCount, regions);

	endSingleTimeCommands(QueueType::Graphics, buffer);
}

void DisjointCommandDispatcher::createTexture(uint32_t  width, uint32_t  height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageType imageType, uint32_t  depth, uint32_t  layerCount, uint32_t mipCount, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlags, VkImageLayout layoutType = VK_IMAGE_LAYOUT_UNDEFINED, bool bindMemory = true) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = depth;
	imageInfo.mipLevels = mipCount;
	imageInfo.arrayLayers = layerCount;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = layoutType;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = createFlags;
	if (vkCreateImage(*device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	if (!bindMemory)
		return;
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(*device, image, &memRequirements);

	uint32_t index = findMemoryType(memRequirements, properties);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = index;

	AllocateMemory(allocInfo, imageMemory);

	vkBindImageMemory(*device, image, imageMemory, 0);
}

void DisjointCommandDispatcher::AllocateMemory(const VkMemoryAllocateInfo& allocInfo, VkDeviceMemory& imageMemory) {
	if (vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
}
/// <summary>
/// Allocates an entire Memory block based on params
/// </summary>
/// <param name="memReq">What memory requirements are needed</param>
/// <param name="size">length of the memory needed</param>
/// <param name="stride">stride of each element</param>
/// <param name="propertyFlags">This returns a refence to the propertyFlags used</param>
/// <param name="MemoryBlock">Memory allocated</param>
void DisjointCommandDispatcher::AllocateMemoryBlock(const VkMemoryRequirements memReq, uint64_t size, uint64_t stride, int32_t& propertyFlags, VkDeviceMemory& MemoryBlock)
{
	//Some platforms limit the maximum size a single allocation
	//shocking few bits...
	uint64_t allocationSize = static_cast<uint64_t>(size) * static_cast<uint64_t>(stride);
	propertyFlags = DisjointCommandDispatcher::dispatcher->findProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (propertyFlags == -1)
	{
		trace_error("Failed to get property flags");
		throw std::exception("Failed to get property flags");
	}
		

		VkMemoryAllocateInfo info{};

		info.allocationSize = allocationSize;
		info.memoryTypeIndex = propertyFlags;
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		DisjointCommandDispatcher::dispatcher->AllocateMemory(info, MemoryBlock);
	
}

bool DisjointCommandDispatcher::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void DisjointCommandDispatcher::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipCount, int layerCount ) {
	VkCommandBuffer buffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	barrier.srcAccessMask = 0; //TODO
	barrier.dstAccessMask = 0; //TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else 	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//isnt working rn
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		buffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	VkFence* fence = DisjointDispatcher->Fences->get();
	endSingleTimeCommands(QueueType::Graphics, buffer, fence);
	DisjointDispatcher->Fences->give(fence);
}

void DisjointCommandDispatcher::DestroySampler(VkSampler sampler, VkAllocationCallbacks* callback)
{
	vkDestroySampler(*device, sampler, callback);
}

void DisjointCommandDispatcher::DestroyImageView(VkImageView imageView, VkAllocationCallbacks* callback)
{
	vkDestroyImageView(*device, imageView, callback);
}

void DisjointCommandDispatcher::DestroyImage(VkImage image, VkAllocationCallbacks* callback)
{
	vkDestroyImage(*device, image, callback);
}

void DisjointCommandDispatcher::FreeMemory(VkDeviceMemory memory, VkAllocationCallbacks* callback)
{
	vkFreeMemory(*device, memory, callback);
}

void DisjointCommandDispatcher::createImageSampler(VkFilter filter, VkSamplerAddressMode addressMode, VkSampler& sampler) {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(*physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = filter;
	samplerInfo.minFilter = filter;//vk_filter_nearest for pixels
	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;
	//mirrored repeat
	//clamp_to_edge (uses colour of edge closest)
	//mirror_clamp_to_edge (uses edge opposite)
	//clamp to border (clamps to border only solid colour after)

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;//either white black or transparent in float or int
	samplerInfo.unnormalizedCoordinates = VK_FALSE; //use true if you want to use 0 to tex width e.g chunk textures

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	//samplerInfo.mipLodBias = 0;
	//samplerInfo.minLod = 0;
	//samplerInfo.maxLod = 0;

	if (vkCreateSampler(*device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler");
}

void DisjointCommandDispatcher::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType type, uint32_t  layerCount, int32_t mips, VkImageView* view) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = type;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mips;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layerCount;

	if (vkCreateImageView(*device, &viewInfo, nullptr, view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

void DisjointCommandDispatcher::resetFence(const VkFence& fence) {
	vkResetFences(*device, 1, &fence);
}


void DisjointCommandDispatcher::GetPhysicalDeviceProperties(VkPhysicalDeviceMemoryProperties& properties)
{
	vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &properties);
}
void DisjointCommandDispatcher::GetPhysicalDeviceProperties(VkPhysicalDeviceProperties2& properties)
{
	vkGetPhysicalDeviceProperties2(*physicalDevice, &properties);
}

void DisjointCommandDispatcher::GetPhysicalDeviceProperties(VkPhysicalDeviceProperties& properties)
{
	vkGetPhysicalDeviceProperties(*physicalDevice, &properties);
}

// Find a memory in `memoryTypeBitsRequirement` that includes all of `requiredProperties`

int32_t DisjointCommandDispatcher::findProperties(uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties)
{
	VkPhysicalDeviceMemoryProperties pMemoryProperties;
	GetPhysicalDeviceProperties(pMemoryProperties);

	const uint32_t memoryCount = pMemoryProperties.memoryTypeCount;

	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
	{
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

		const VkMemoryPropertyFlags properties = pMemoryProperties.memoryTypes[memoryIndex].propertyFlags;
		const bool hasRequiredProperties = (properties & requiredProperties) == requiredProperties;

		if (isRequiredMemoryType && hasRequiredProperties)
		{
			return static_cast<int32_t>(memoryIndex);
		}
	}

	// failed to find memory type
	return -1;
}

void DisjointCommandDispatcher::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo{};

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements, properties);

	if (vkAllocateMemory(*device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer memory");

	vkBindBufferMemory(*device, buffer, bufferMemory, 0);
}
/// <summary>
/// 
/// </summary>
/// <param name="buffer"></param>
/// <param name="bufferMemory"></param>
/// <param name="offset"></param> Start of the memory to be bound. Offset from the start of buffer memory
void DisjointCommandDispatcher::BindBufferMemory(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize offset = 0) {
	vkBindBufferMemory(*device, buffer, bufferMemory, offset);
}
void DisjointCommandDispatcher::SetTextureFromPath(char* path, VkImage image, VkDeviceMemory textureImageMemory) {
	int texWidth, texHeight, texChannels;

	int dirRes = IsDirectoryOrFile(path);



	std::vector<stbi_uc*> pixeles = std::vector<stbi_uc*>();
	if (dirRes == 0) {
		std::vector<std::string> paths = GetAllFilesInDirectory(path);

		for (size_t i = 0; i < paths.size(); i++)
		{
			pixeles.push_back(stbi_load(paths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha));

			if (!pixeles[i])
				throw std::runtime_error("faied to load texture image!");

		}
	} else if (dirRes ==1)
	{
		pixeles.push_back(stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha));

		if (!pixeles[0])
			throw std::runtime_error("faied to load texture image!");
	}

	//stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);


	VkDeviceSize imageSize = texWidth * texHeight * 4;


	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, pixeles.size());

	for (size_t i = 0; i < pixeles.size(); i++)
	{


	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixeles[i], static_cast<size_t>(imageSize));
	vkUnmapMemory(*device, stagingBufferMemory);

	stbi_image_free(pixeles[i]);


	copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), i,1);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);

	}
	transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, pixeles.size());

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
}


void DisjointCommandDispatcher::CopyArrayToTexture(void* arrayData, int stride, VkImage image, VkDeviceMemory textureImageMemory, uint32_t  width, uint32_t  height, uint32_t layer, int layerOffset) {


	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VkDeviceSize imageSize = width * height * stride * layer;


	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	int size = width * height * layer * stride;

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, size, 0, &data);
	memcpy(data, arrayData, static_cast<size_t>(size));
	vkUnmapMemory(*device, stagingBufferMemory);



	copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), layerOffset, layer);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);

}



void DisjointCommandDispatcher::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t  width, uint32_t  height, uint32_t layer, uint32_t layerCount = 1) {
	VkCommandBuffer commandBuffer = invokeSingleTimeCommands(*device,*commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = layer;
	region.imageSubresource.layerCount = layerCount;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};
	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	auto fence = Fences->get();
	endSingleTimeCommands(QueueType::Graphics, commandBuffer, fence);
	Fences->give(fence);
}

//void DisjointCommandDispatcher::WriteIDGPUpdates(const BoundMemoryPacket& packet, void* pushData, int pushAmount, int pushStride, void* ChangeData, int changeStride, int changeAmount, DescriptiveObject& buffer) {
//	//buffer->setBufferData(push.data(), packet.packet.index, push.size() * sizeof(IDGPUPush));
//	ComputeBufferInfo modInfo{};
//	ComputeBufferInfo limitsMod{};
//
//	modInfo.bufferSize = pushAmount;
//	modInfo.stride = pushStride;
//
//	limitsMod.bufferSize = changeAmount;
//	limitsMod.stride = changeStride;
//
//
//	ComputeBuffer* limitsb = new ComputeBuffer(limitsMod);
//	ComputeBuffer* modifications = new ComputeBuffer(modInfo);
//
//	limitsb->setBufferData(ChangeData);
//	modifications->setBufferData(pushData);
//
//
//	WriteUpdatesToBufferConst wc{};
//
//	wc.count = limitsMod.bufferSize;
//	wc.index = packet.getIndex(sizeof(float));
//	wc.sizeofdata = pushStride;
//
//
//	VkFence* fence = DisjointCommandDispatcher::dispatcher->Fences->get();
//	Shaders::Shaders[Shaders::CopyWorldToBuffer]->dispatch(1, 1, ceil(wc.count / 16.0),
//		sizeof(WriteUpdatesToBufferConst), &wc, fence,
//		MonoidList(3).bind(modifications)->bind(limitsb)->bind(buffer)->render());
//
//
//	DisjointCommandDispatcher::dispatcher->Fences->give(fence);
//
//
//	modifications->Deallocate();
//	limitsb->Deallocate();
//}


VkCommandPool DisjointCommandDispatcher::createCommandPool(QueueType type) {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(*physicalDevice, *surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		switch (type)
		{
		case QueueType::Compute:
			poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
			break;
		case QueueType::Graphics:
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
			break;
		default:
			break;
		}



		VkCommandPool commandPool{};
		if (vkCreateCommandPool(*device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}

		return commandPool;
	}
VkCommandBuffer DisjointCommandDispatcher::createCommandBuffers(VkCommandPool pool, VkCommandBufferLevel level)
{

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = 1;
	//allocInfo.commandBufferCount = (uint32_t )commandBuffers.size();
	VkCommandBuffer buffer{};
	if (vkAllocateCommandBuffers(*device, &allocInfo, &buffer) != VK_SUCCESS)
		throw  std::runtime_error("failed to allocate command buffers");

	return buffer;
}

void DisjointCommandDispatcher::createFrameBuffer(VkFramebufferCreateInfo framebufferInfo, VkFramebuffer* buffer)
{

	if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}


void DisjointCommandDispatcher::DestroyCommandPool(VkCommandPool& pool) {
	vkDestroyCommandPool(*device, pool, nullptr);
}
void DisjointCommandDispatcher::DestroyFrameBuffer(VkFramebuffer& buffer) {
	vkDestroyFramebuffer(*device, buffer, nullptr);
}

VkRenderPass DisjointCommandDispatcher::CreateRenderPass(const VkRenderPassCreateInfo& createInfo) {
	VkRenderPass pass{};

	if (vkCreateRenderPass(*device, &createInfo, nullptr, &pass) != VK_SUCCESS)
		throw  std::runtime_error("failed to create render pass");
	return pass;
}
VkRenderPass DisjointCommandDispatcher::CreateRenderPass2(const VkRenderPassCreateInfo2& createInfo)
{
	VkRenderPass pass{};

	if (vkCreateRenderPass2(*device, &createInfo, nullptr, &pass) != VK_SUCCESS)
		throw  std::runtime_error("failed to create render pass");
	return pass;
}
void DisjointCommandDispatcher::DestroyRenderPass(const VkRenderPass& renderpass) {
	vkDestroyRenderPass(*device, renderpass, nullptr);
}
void DisjointCommandDispatcher::ResetCommandPoolMemory(const VkCommandPool& pool) {
	vkResetCommandPool(*device, pool, 0);
}

void DisjointCommandDispatcher::CreateAccelerationStructure(const VkAccelerationStructureCreateInfoKHR* pCreateInfo, VkAccelerationStructureKHR& pAccelerationStructure)
{
	Handle_vkCreateAccelerationStructureKHR(*device, pCreateInfo, nullptr, &pAccelerationStructure);
}

void DisjointCommandDispatcher::DestroyAccelerationStructure(VkAccelerationStructureKHR& pAccelerationStructure)
{
	Handle_vkDestroyAccelerationStructureKHR(*device, pAccelerationStructure, nullptr);
}

void DisjointCommandDispatcher::GetAccelerationStructureInfo(const VkAccelerationStructureBuildGeometryInfoKHR* pCreateInfo, const uint32_t maxCount, VkAccelerationStructureBuildSizesInfoKHR& pSizeInfo)
{
	Handle_vkGetAccelerationStructureBuildSizesKHR(*device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, pCreateInfo, &maxCount, &pSizeInfo);
}

VkDeviceAddress DisjointCommandDispatcher::GetBufferDeviceAddress(const VkBufferDeviceAddressInfo& info)
{
	return vkGetBufferDeviceAddress(*device, &info);
}

VkDeviceAddress DisjointCommandDispatcher::GetAccelerationDeviceAddress(const VkAccelerationStructureDeviceAddressInfoKHR& info)
{
	return Handle_vkGetAccelerationStructureDeviceAddressKHR(*device, &info);
}


void DisjointCommandDispatcher::createShaderModule(const std::string path, VkShaderModule* shaderModule) {

	auto code = readShaderFile(path);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(*device, &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		throw std::exception("failed  to create shader module");
	}
}		

void DisjointCommandDispatcher::DestroyShaderModule(VkShaderModule& shaderModule) {
	vkDestroyShaderModule(*device, shaderModule, nullptr);
}		

void DisjointCommandDispatcher::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info, VkDescriptorSetLayout& layout) {
	if (vkCreateDescriptorSetLayout(*device, &info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}
void DisjointCommandDispatcher::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& info, VkPipelineLayout& layout) {
	if (vkCreatePipelineLayout(*device, &info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
}
void DisjointCommandDispatcher::CreateRaytracingPipeline(const VkRayTracingPipelineCreateInfoKHR& info, VkPipeline& pipeline) {
	if (Handle_vkCreateRayTracingPipelinesKHR(*device, {}, {}, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create raytracing pipeline!");
}

void DisjointCommandDispatcher::DestroyPipelineLayout(VkPipelineLayout& layout) {
	vkDestroyPipelineLayout(*device, layout, nullptr);
}

void DisjointCommandDispatcher::DestroyPipeline(VkPipeline& pipeline) {
	vkDestroyPipeline(*device, pipeline, nullptr);
}

void DisjointCommandDispatcher::GetRaytracingShaderHandles(void* data, const VkPipeline& pipeline, uint32_t handleCount, uint32_t dataSize) {
	if (Handle_vkGetRayTracingShaderGroupHandlesKHR(*device, pipeline, 0, handleCount, dataSize, data) != VK_SUCCESS)
		throw std::runtime_error("failed to get raytracing shader handles");
}