#include "ProductionPackage.h"
#include "DisjointCommandDispatcher.h"

nve::ProductionPackage::ProductionPackage(QueueType type)
{
	CommandDispatcher->createFence(&fence);
	this->type = type;
	//auto cmd = CommandDispatcher->createCommandPool();
	commandpool = (VkCommandPool*)malloc(sizeof(VkCommandPool));
	*commandpool = CommandDispatcher->createCommandPool(type);
	auto buffer = CommandDispatcher->createCommandBuffers(*commandpool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	primaryCommandBuffer = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer));
	*primaryCommandBuffer = buffer;
}

VkCommandBuffer nve::ProductionPackage::CreateSecondaryCommandBuffer() const
{
	auto cmd = CommandDispatcher->createCommandBuffers(*commandpool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkRenderingInfoKHR render_info{ VK_STRUCTURE_TYPE_RENDERING_INFO_KHR };
	vkCmdBeginRendering(cmd, &render_info);
	return cmd;
}
nve::ProductionPackage::~ProductionPackage()
{

	// Destroy fence
	CommandDispatcher->destroyFence(fence);
	// Destroy pool
	CommandDispatcher->DestroyCommandPool(*commandpool);

	free(commandpool);
	// Queue isnt owned by this object
	// Buffers get dellocated by destroying the pool
}