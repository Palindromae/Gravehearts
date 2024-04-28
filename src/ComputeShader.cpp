#include "stdafx.h"
#include "ComputeShader.h"
#include "QueueTypes.h"
#include "DescriptiveObject.h"
ComputeShader::ComputeShader(VkDevice& aDevice, std::vector<VkDescriptorSetLayoutBinding>& bindings, int pushConstantSize, std::string path) {
	DisjointDispatcher->createShaderModule(path, &shaderModule);

	/*
	VkDescriptorSetLayoutBinding bindings[2] = { {0} };
	//custom
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	bindings[1].binding = 0;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	*/

	//	std::vector< VkDescriptorSetLayoutCreateInfo> layoutSetup{};

	for (size_t i = 0; i < bindings.size(); i++)
	{
		VkDescriptorSetLayout layoutT{};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};

		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};

		//if the max binding is 1 then most likely this isnt a dynamicly bound object or shouldnt be at least
		if (bindings[i].descriptorCount > 1) {
			VkDescriptorBindingFlags flags;
			flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
			flagsInfo.pBindingFlags = &flags;
			flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			flagsInfo.bindingCount = 1;
			layoutInfo.pNext = &flagsInfo;
		}

		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &bindings[i];
		layoutInfo.flags = 0;
		//layoutSetup.push_back(layoutInfo);

		if (vkCreateDescriptorSetLayout(aDevice, &layoutInfo, nullptr, &layoutT) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");

		setLayout.push_back(layoutT);
	}

	VkPushConstantRange pushRange{};
	pushRange.offset = 0;
	pushRange.size = pushConstantSize;
	pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkPipelineLayoutCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.flags = 0;
	info.setLayoutCount = setLayout.size();
	info.pSetLayouts = setLayout.data();
	info.pushConstantRangeCount = 1;
	info.pPushConstantRanges = &pushRange;
	info.pNext = nullptr;

	if (vkCreatePipelineLayout(aDevice, &info, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");

	VkComputePipelineCreateInfo cInfo{};

	cInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	cInfo.pNext = 0;
	cInfo.flags = 0;
	cInfo.stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, 0,0,VK_SHADER_STAGE_COMPUTE_BIT,shaderModule,"main",0 };

	cInfo.layout = pipelineLayout;
	cInfo.basePipelineHandle = 0;
	cInfo.basePipelineIndex = 0;

	if (vkCreateComputePipelines(aDevice, VK_NULL_HANDLE, 1, &cInfo, nullptr, &pipe) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute shader");
	}

	vkDestroyShaderModule(aDevice, shaderModule, nullptr);
}
VkCommandBuffer ComputeShader::dispatch(const int x, const int y, const int z, const int pushSize, const void* pushConstants, VkFence* fence, const std::vector<VkDescriptorSet>& buffers, QueueType type, const bool waitOnFence) const
{
	VkCommandBuffer commandBuffer = DisjointCommandDispatcher::dispatcher->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, buffers.size(), buffers.data(), 0, nullptr);

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushSize, pushConstants);

	vkCmdDispatch(commandBuffer, x, y, z);

	if (fence == nullptr && waitOnFence)
 		trace_debug("no fence");

	return DisjointCommandDispatcher::dispatcher->endSingleTimeCommands(type, commandBuffer, fence, waitOnFence);
}

VkCommandBuffer ComputeShader::dispatch(nve::ProductionPackage* context, const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers, const bool waitOnFence) const
{
	VkCommandBuffer commandBuffer = DisjointCommandDispatcher::dispatcher->beginSingleTimeCommands(context);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, buffers.size(), buffers.data(), 0, nullptr);

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushSize, pushConstants);

	vkCmdDispatch(commandBuffer, x, y, z);

	if (context->fence == nullptr && waitOnFence)
		trace_debug("no fence");

	return DisjointCommandDispatcher::dispatcher->endSingleTimeCommands(context, commandBuffer, waitOnFence);
}

VkCommandBuffer ComputeShader::dispatchIndirect(nve::ProductionPackage* context, const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers, const bool waitOnFence) const
{
	VkCommandBuffer commandBuffer = DisjointCommandDispatcher::dispatcher->beginSingleTimeCommands(context);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, buffers.size(), buffers.data(), 0, nullptr);

	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushSize, pushConstants);

	vkCmdDispatchIndirect(commandBuffer, buffer->buffer, offset);
	//vkCmdDispatch(commandBuffer, x, y, z);

	if (context->fence == nullptr && waitOnFence)
		trace_debug("no fence");

	return DisjointCommandDispatcher::dispatcher->endSingleTimeCommands(context, commandBuffer, waitOnFence);
}

 DeferedShader ComputeShader::defer(const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers) const
 {

	  return [this, x, y, z, pushSize, pushConstants, buffers](VkCommandBuffer commandBuffer) {

		  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

		  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, buffers.size(), buffers.data(), 0, nullptr);

		  vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushSize, pushConstants);

		  vkCmdEndRendering(commandBuffer);

		  vkCmdDispatch(commandBuffer, x, y, z);

	  };
 }

 DeferedShader ComputeShader::deferIndirect(const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers) const
 {
	 return [this, offset, buffer, pushSize, pushConstants, buffers](VkCommandBuffer commandBuffer) {

		 vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

		 vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, buffers.size(), buffers.data(), 0, nullptr);

		 vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushSize, pushConstants);

		 vkCmdEndRendering(commandBuffer);

		 vkCmdDispatchIndirect(commandBuffer, buffer->buffer, offset);

	};	
 }



