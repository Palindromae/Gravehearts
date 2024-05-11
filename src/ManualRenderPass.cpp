#include "ManualRenderPass.h"
#include "DisjointCommandDispatcher.h"


nve::ManualRenderPass::ManualRenderPass(nve::ProductionPackage* context) : context(context)
{
	VkCommandBufferBeginInfo begin
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};
	vkBeginCommandBuffer(*context->primaryCommandBuffer, &begin);
}

// VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT STAGE MASK
// VK_ACCESS_INDIRECT_COMMAND_READ_BIT ACCESS MASK
void nve::ManualRenderPass::InsertMemoryBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkMemoryBarrier memoryBarrier = {
   .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
   .srcAccessMask = srcAccessMask, // Accesses that must be completed before the barrier
   .dstAccessMask = dstAccessMask, // Accesses that must wait for the barrier
	};

	vkCmdPipelineBarrier(*context->primaryCommandBuffer, srcStage, dstStage, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}

void nve::ManualRenderPass::InlineShaderDispatch(Shaders::ShaderClass shader, const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers)
{
	VkRenderingInfoKHR render_info{
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
	.layerCount = 1,
	};
	render_info.renderArea.extent = VkExtent2D(1, 1);
	vkCmdBeginRendering(*context->primaryCommandBuffer, &render_info);

	Shaders::Shaders[shader]->inlineDispatch(*context->primaryCommandBuffer, x, y, z, pushSize, pushConstants, buffers);
}

void nve::ManualRenderPass::InlineShaderIndirect(Shaders::ShaderClass shader, const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers)
{

	VkRenderingInfoKHR render_info{
	.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
	.layerCount = 1,
	};
	render_info.renderArea.extent = VkExtent2D(1, 1);
	vkCmdBeginRendering(*context->primaryCommandBuffer, &render_info);

	Shaders::Shaders[shader]->inlineIndirect(*context->primaryCommandBuffer, buffer, offset, pushSize, pushConstants, buffers);
}


void nve::ManualRenderPass::execute()
{
	vkEndCommandBuffer(*context->primaryCommandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = context->primaryCommandBuffer;

	CommandDispatcher->queues->dispatch(context->type, submitInfo, context->fence);

	DisjointDispatcher->WaitOnOneFenceMax(&context->fence);
	DisjointDispatcher->resetFence(context->fence);
	vkResetCommandBuffer(*context->primaryCommandBuffer, 0);
}

nve::ManualRenderPass::~ManualRenderPass()
{
}
