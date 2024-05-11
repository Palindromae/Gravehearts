#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <string>
#include "DisjointCommandDispatcher.h"
#include "ProductionPackage.h"
#include "NveTypes.h"
#include "ComputeBuffer.h"

#define SizeConstants(A) sizeof(A), &A

class ComputeShader {

public:
	VkShaderModule shaderModule{};
	std::vector<VkDescriptorSetLayout> setLayout{};

	VkPipelineLayout pipelineLayout{};
	VkPipeline pipe{};

	ComputeShader(VkDevice& aDevice, std::vector<VkDescriptorSetLayoutBinding>& bindings, int pushConstantSize, std::string path);
	~ComputeShader();


	// Dispatch
	VkCommandBuffer dispatch(const int x, const int y, const int z, const int pushSize, const void* pushConstants, VkFence* fence, const std::vector<VkDescriptorSet>& buffers = {}, QueueType type = QueueType::Graphics, const bool waitOnFence = true) const;
	VkCommandBuffer dispatch(nve::ProductionPackage* context, const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers = {}, const bool waitOnFence = true) const;
	VkCommandBuffer dispatchIndirect(nve::ProductionPackage* context, const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers = {}, const bool waitOnFence = true) const;

	// Defer
	DeferedShader defer(const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers = {}) const;
	DeferedShader deferIndirect(const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers = {}) const;

	// Inline
	void inlineDispatch(const VkCommandBuffer& commandBuffer, const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers) const;
	void inlineIndirect(const VkCommandBuffer& commandBuffer, const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers) const;
};