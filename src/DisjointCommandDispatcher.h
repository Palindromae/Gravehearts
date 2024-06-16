#pragma once
#include <vulkan/vulkan_core.h>
#include "SingleTimeCommands.h"
#include <stdexcept>
#include "SpareFences.h"
//#include "BoundMemoryPacket.h"
#include "DescriptiveObject.h"
#include "ProductionPackage.h"
#include "nveQueues.h"
#include "nvvk/memallocator_dma_vk.hpp"
#include "nvvk/resourceallocator_vk.hpp"
#include <nvvk/vulkanhppsupport.hpp>

#define DisjointDispatcher DisjointCommandDispatcher::dispatcher
#define CommandDispatcher DisjointCommandDispatcher::dispatcher
class DisjointCommandDispatcher {


private:
	int NoSpareFences = 4;
	
	PFN_vkGetRayTracingShaderGroupHandlesKHR Handle_vkGetRayTracingShaderGroupHandlesKHR{};
	PFN_vkGetAccelerationStructureBuildSizesKHR Handle_vkGetAccelerationStructureBuildSizesKHR{};
	PFN_vkGetAccelerationStructureDeviceAddressKHR Handle_vkGetAccelerationStructureDeviceAddressKHR{};
	PFN_vkCreateRayTracingPipelinesKHR Handle_vkCreateRayTracingPipelinesKHR{};
	PFN_vkCreateAccelerationStructureKHR Handle_vkCreateAccelerationStructureKHR{};
	PFN_vkDestroyAccelerationStructureKHR Handle_vkDestroyAccelerationStructureKHR{};

public:
	static inline  DisjointCommandDispatcher* dispatcher;
	PFN_vkCmdTraceRaysKHR Handle_vkCmdTraceRaysKHR{};
	PFN_vkCmdBuildAccelerationStructuresKHR Handle_vkCmdBuildAccelerationStructuresKHR{};
	PFN_vkCmdBeginRenderingKHR Handle_vkCmdBeginRenderingKHR{};
	nvvk::ResourceAllocatorDma* m_alloc;

	SpareFence* Fences;

	DisjointCommandDispatcher(VkDevice* device, VkCommandPool* pool, VkQueue* graphicsQ, VkQueue* presentationQ, VkQueue* computeQ, VkPhysicalDevice* physicalDevice,VkSurfaceKHR*  surface, nvvk::ResourceAllocatorDma* m_alloc) : m_alloc(m_alloc) {
		dispatcher = this;
		this->device = device;
		commandPool = pool;

		std::vector<VkQueue*> queues_vctr = { computeQ, presentationQ, graphicsQ  };

		queues = new NveQueues(queues_vctr);

		this->physicalDevice = physicalDevice;
		Fences = new SpareFence(NoSpareFences);
		this->surface = surface;

		Handle_vkCreateAccelerationStructureKHR        = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(*device, "vkCreateAccelerationStructureKHR"));
		Handle_vkCmdBuildAccelerationStructuresKHR     = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(*device, "vkCmdBuildAccelerationStructuresKHR"));
		Handle_vkGetRayTracingShaderGroupHandlesKHR    = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(*device, "vkGetRayTracingShaderGroupHandlesKHR"));
		Handle_vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(*device, "vkGetAccelerationStructureBuildSizesKHR"));
		Handle_vkCreateRayTracingPipelinesKHR          = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(*device, "vkCreateRayTracingPipelinesKHR"));
		Handle_vkCmdTraceRaysKHR					   = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(*device, "vkCmdTraceRaysKHR"));
		Handle_vkDestroyAccelerationStructureKHR       = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(*device, "vkDestroyAccelerationStructureKHR"));
		Handle_vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(*device, "vkGetAccelerationStructureDeviceAddressKHR"));
		Handle_vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(*device, "vkCmdBeginRenderingKHR"));
	}

	void createFence(VkFence* fence, VkFenceCreateFlagBits flag = VkFenceCreateFlagBits(0));

	void CreateDescriptorPool(VkDescriptorPoolCreateInfo& poolInfo, VkDescriptorPool& pool);

	void BindSparseMemory(VkBindSparseInfo& info, const VkFence& fence);
	void GetImageRequirements(const VkImage& image, uint32_t* count, VkSparseImageMemoryRequirements& req);

	void CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& layoutInfo, VkDescriptorSetLayout& layout);

	void createDescriptorSets(const VkDescriptorSetAllocateInfo& allocInfo, VkDescriptorSet& set);

	void updateDescriptorSets(int writeCount, VkWriteDescriptorSet& descriptorWrites, uint32_t descriptorCopyCount);

	void updateDescriptorSets(int writeCount, VkWriteDescriptorSet* descriptorWrites, uint32_t descriptorCopyCount);

	//Buffer Commands
	void createBuffer(const VkBufferCreateInfo& bufferInfo, VkBuffer& buffer);
	uint32_t findMemoryType(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties);
	void AllocateMemory(VkMemoryPropertyFlags& properties, VkDeviceMemory& memory, bool requires_exact_device_match);
	bool GenerateMemoryAllocateInfo(const VkMemoryRequirements& req, VkMemoryPropertyFlags& properties, VkDeviceMemory& bufferMemory, bool requires_exact_device_match, VkMemoryAllocateInfo& allocInfo);
	VkMemoryRequirements GetBufferMemoryRequirements(const VkBuffer& buffer);

	bool allocateBufferMemory(const VkBuffer& buffer, VkMemoryPropertyFlags& properties, const VkMemoryPropertyFlags& memflags, VkDeviceMemory& bufferMemory, bool requires_exact_device_match);

	void setStagingBuffer(VkDeviceMemory& stagingBufferMemory, VkDeviceSize bufferSize, const void* dataArray);

	void MapMemory(VkDeviceMemory* memory, VkDeviceSize length, void*& data);

	void UnmapMemory(VkDeviceMemory* memory);

	void copyDataToBuffer(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, VkDeviceSize offset, const void* data);

	void copyDataToBuffer(VkBuffer& stagingBuffer, VkDeviceMemory stagingBufferMemory, VkBuffer& buffer, VkDeviceSize offset, VkDeviceSize bufferSize, nve::ProductionPackage* context = nullptr, VkFence* fence = nullptr);


	void copyBufferToBuffer(VkBuffer& src, VkBuffer& dst, uint32_t regions, const VkBufferCopy* copies, nve::ProductionPackage* context = nullptr);

	void CopyMemoryDirect(VkDeviceSize offset, VkDeviceSize bufferSize, void* dataOut, nve::ProductionPackage* context);


	void copyDataFromBuffer(VkBuffer& stagingBuffer, VkDeviceMemory stagingBufferMemory, VkBuffer& buffer, VkDeviceSize bufferSize, VkDeviceSize offset, void* dataOut, nve::ProductionPackage* context = nullptr);

	void destroyBuffer(VkBuffer& buffer);

	void destroyBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkDescriptorPool& pool, VkDescriptorSetLayout& layout);

	void destroyDescriptors(VkDescriptorPool& pool, VkDescriptorSetLayout& layout);

	void destroyLayouts(VkDescriptorSetLayout* layout, int count);

	void blitImage(VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout, int regionCount, VkImageBlit* regions, VkFilter filter, VkFence* fence = VK_NULL_HANDLE, const bool resetFence = true);


	VkCommandBuffer endSingleTimeCommands(QueueType type, VkCommandBuffer commandBuffer, VkFence* fence = VK_NULL_HANDLE, const bool WaitOnFence = true, const bool ResetFence = true);
	void WaitTilIdle(nve::ProductionPackage* context);
	VkCommandBuffer endSingleTimeCommands(nve::ProductionPackage*  context, VkCommandBuffer commandBuffer, const bool WaitOnFence = true, const bool ResetFence = true);

	void WaitOnOneFenceMax(const VkFence* fence);

	void destroyFence(VkFence& fence);

	//Images
//	void blitImage(VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout, int regionCount, VkImageBlit* regions, VkFilter filter);

	void copyImage(VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout, int regionCount, const VkImageCopy* regions);

	void createTexture(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageType imageType, uint32_t depth, uint32_t layerCount, uint32_t mipCount, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlags, VkImageLayout layoutType, bool bindMemory);

	void AllocateMemory(const VkMemoryAllocateInfo& allocInfo, VkDeviceMemory& imageMemory);
	void AllocateMemoryBlock(const VkMemoryRequirements req, uint64_t size, uint64_t stride, int32_t& propertyFlags, VkDeviceMemory& MemoryBlock);

	bool hasStencilComponent(VkFormat format);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipCount, int layerCount = 1);


	void DestroySampler(VkSampler sampler, VkAllocationCallbacks* callback = nullptr);
	void DestroyImage(VkImage image, VkAllocationCallbacks* callback = nullptr);
	void DestroyImageView(VkImageView imageView, VkAllocationCallbacks* callback = nullptr);
	void FreeMemory(VkDeviceMemory memory, VkAllocationCallbacks* callback = nullptr);

	void createImageSampler(VkFilter filter, VkSamplerAddressMode addressMode, VkSampler& sampler);

	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType type, uint32_t  layerCount, int32_t mips, VkImageView* view);

	void resetFence(const VkFence& fence) ;

	void GetPhysicalDeviceProperties(VkPhysicalDeviceMemoryProperties& properties);

	void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties2& properties);

	void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties& properties);

	// Find a memory in `memoryTypeBitsRequirement` that includes all of `requiredProperties`
	int32_t findProperties(
		uint32_t memoryTypeBitsRequirement,
		VkMemoryPropertyFlags requiredProperties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void BindBufferMemory(VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize offset);
	void SetTextureFromPath(char* path, VkImage image, VkDeviceMemory textureImageMemory);
	void CopyArrayToTexture(void* arrayData, int stride, VkImage image, VkDeviceMemory textureImageMemory, uint32_t width, uint32_t height, uint32_t layer, int layerOffset = 0);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer, uint32_t layerCount);

	//void WriteIDGPUpdates(const BoundMemoryPacket& packet, void* pushData, int pushAmount, int pushStride, void* ChangeData, int changeStride, int changeAmount, DescriptiveObject& buffer);

	VkCommandPool createCommandPool(QueueType type = QueueType::Graphics);

	VkCommandBuffer createCommandBuffers(VkCommandPool pool, VkCommandBufferLevel level);


	void createFrameBuffer(VkFramebufferCreateInfo framebufferInfo, VkFramebuffer* buffer);


	void DestroyCommandPool(VkCommandPool& pool);

	void DestroyFrameBuffer(VkFramebuffer& buffer);


	VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo& createInfo);
	VkRenderPass CreateRenderPass2(const VkRenderPassCreateInfo2& createInfo);

	void DestroyRenderPass(const VkRenderPass& renderpass);

	void ResetCommandPoolMemory(const VkCommandPool& pool);

	void CreateAccelerationStructure(const VkAccelerationStructureCreateInfoKHR* pCreateInfo, VkAccelerationStructureKHR& pAccelerationStructure);
	void DestroyAccelerationStructure(VkAccelerationStructureKHR& pAccelerationStructure);
	void GetAccelerationStructureInfo(const VkAccelerationStructureBuildGeometryInfoKHR* pCreateInfo, const uint32_t maxCount, VkAccelerationStructureBuildSizesInfoKHR& pSizeInfo);
	 
	VkDeviceAddress GetBufferDeviceAddress(const VkBufferDeviceAddressInfo& info);

	VkDeviceAddress GetAccelerationDeviceAddress(const VkAccelerationStructureDeviceAddressInfoKHR& info);

	void createShaderModule(const std::string path, VkShaderModule* shaderModule);

	void DestroyShaderModule(VkShaderModule& shaderModule);

	void CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info, VkDescriptorSetLayout& layout);

	void CreatePipelineLayout(const VkPipelineLayoutCreateInfo& info, VkPipelineLayout& layout);

	void CreateRaytracingPipeline(const VkRayTracingPipelineCreateInfoKHR& info, VkPipeline& pipeline);

	void DestroyPipelineLayout(VkPipelineLayout& layout);

	void DestroyPipeline(VkPipeline& pipeline);

	void GetRaytracingShaderHandles(void* data, const VkPipeline& pipeline, uint32_t handleCount, uint32_t dataSize);

	NveQueues* queues;

	VkDescriptorPool CreateDescriptorPool(const nvvk::DescriptorSetBindings& nvvkBindings) {
		return nvvkBindings.createPool(*device);
	}

	VkDescriptorSetLayout CreateDescriptorSetLayout(nvvk::DescriptorSetBindings& nvvkBindings) {
		return nvvkBindings.createLayout(*device);
	}


	VkCommandBuffer beginSingleTimeCommands(nve::ProductionPackage* package) {
		return invokeSingleTimeCommands(*device, *package->commandpool);
	}

	VkCommandBuffer beginSingleTimeCommands() {
		return invokeSingleTimeCommands(*device, *commandPool);
	}
private:

	VkDevice* device;
	VkCommandPool* commandPool;



	VkPhysicalDevice* physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR* surface;
};