#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include "ProductionPackage.h"
#include "nveTypes.h"
#include "StoredComputeShaders.h"

namespace nve {

	class ManualRenderPass {
		nve::ProductionPackage* context;

	public:
		ManualRenderPass(nve::ProductionPackage* context);
		void execute();
		~ManualRenderPass();
		void InsertMemoryBarrier(VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VkAccessFlags srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT, VkAccessFlags dstAccessMask = VK_ACCESS_MEMORY_READ_BIT);
		void InlineShaderDispatch(Shaders::ShaderClass shader, const int x, const int y, const int z, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers);
		void InlineShaderIndirect(Shaders::ShaderClass shader, const ComputeBuffer* buffer, const int offset, const int pushSize, const void* pushConstants, const std::vector<VkDescriptorSet>& buffers);
	};

}