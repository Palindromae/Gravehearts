#include "RenderPassGenerator.h"
#include "DisjointCommandDispatcher.h"
#include <execution>
#include <vulkan/vulkan_core.h>

nve::RenderPassGenerator::RenderPassGenerator(int subpassesCount)
{
	subpasses.reserve(subpassesCount);
}

nve::RenderPassGenerator::~RenderPassGenerator()
{
	for (size_t i = 0; i < dependancies.size(); i++)
	{
		delete[] dependancies[i];
	}
}

int nve::RenderPassGenerator::insert(DeferedShader secondary, int dependancy_count, int* dependancies)
{
	subpasses.push_back(secondary);

	int* dependants = (int*)malloc(sizeof(int) * (dependancy_count + 1));

	dependants[0] = dependancy_count;
	memcpy_s(&dependants[1], sizeof(int) * dependancy_count, dependancies, sizeof(int) * dependancy_count);// might be dangerous
	//	for (size_t i = 0; i < dependancy_count; i++)
	//	{
		//	dependancies[1 + i] = dependancies[i];
		//}

	this->dependancies.emplace_back(dependants);
	return index++;
}

/// <summary>
/// Generates a render pass onto the context. A fence must be used to destroy the renderpass afterwards
/// </summary>
/// <param name="context"></param>
void nve::RenderPassGenerator::execute(const ProductionPackage* context)
{

	if (subpasses.size() == 0)
		return;

	auto primary = context->primaryCommandBuffer;


	VkRenderingInfoKHR render_info{
	 .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
		 .layerCount = 1,
	};
	render_info.renderArea.extent = VkExtent2D(1,1);

	VkCommandBufferBeginInfo begin
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};


	vkBeginCommandBuffer(*primary, &begin);

	std::unordered_map<int, int> accepted{};
	bool hasAlloc = true;

	VkMemoryBarrier memoryBarrier = {
	   .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
	   .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT, // Accesses that must be completed before the barrier
	   .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT, // Accesses that must wait for the barrier

	};


	int n = 0;
	while (hasAlloc) {
		hasAlloc = false;

		for (size_t i = 0; i < dependancies.size(); i++)
		{
			if (accepted.contains(i))
				continue;

			int dependants = dependancies[i][0];

			if (dependants == 0)
			{
				// accept
				accepted[i] = n;

				vkCmdBeginRendering(*primary, &render_info);
				subpasses[i](*primary);

				hasAlloc = true;
				continue;
			}

			bool subpassAccepted = false;
			//	std::vector<int> d{};
			//	d.reserve(dependants);
			//	for (size_t j = 1; j < dependants + 1; j++)
			//	{
			////		d.push_back(dependancies[i][j]);
			//	}
			for (size_t j = 1; j < dependants + 1; j++)
			{


				int test = dependancies[i][j];
				if (accepted.contains(dependancies[i][j]) && accepted[dependancies[i][j]] < n) {
					int a = accepted[dependancies[i][j]];
					subpassAccepted = true;
					continue;
				}

				subpassAccepted = false;
				break;
			}

			if (subpassAccepted) {
				accepted[i] = n;
				vkCmdBeginRendering(*primary, &render_info);
				subpasses[i](*primary);
				hasAlloc = true;
			}

		}




		if (accepted.size() == dependancies.size())
			break;
		vkCmdPipelineBarrier(*primary, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
		n++;
	}

	if (accepted.size() != dependancies.size()) {
		throw std::exception("There is a dependancy loop");
	}

	vkEndCommandBuffer(*primary);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = primary;

	CommandDispatcher->queues->dispatch(context->type, submitInfo, context->fence);

	DisjointDispatcher->WaitOnOneFenceMax(&context->fence);
	DisjointDispatcher->resetFence(context->fence);
	vkResetCommandBuffer(*primary, 0);
	//DisjointDispatcher->ResetCommandPoolMemory(*context->commandpool);


}
