#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include "DescriptiveObject.h"
#include "DisjointCommandDispatcher.h"
#include "PagedBinding.h"

struct DescriptorPosition {
	VkDescriptorType type;
	int position;
	int maxCount;
	DescriptorPosition(VkDescriptorType t, int p, int c = 1) : type(t), position(p), maxCount(c) {
	}
};


enum class ComputeBufferType {
	Static,
	Indirect,
	Append,
	Sparse,
	Resident
};

struct ComputeBufferInfo {

	ComputeBufferType type = ComputeBufferType::Static;
	int stride{ 0 };
	int length{ 0 };
	int binding{ 0 };
	VkDeviceSize			bufferSize{ 0 };
	bool					requires_exact_device_match{ true };
	VkBufferUsageFlags		usage{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT}; //| VK_BUFFER_USAGE_SPARSE_BIT
	VkMemoryPropertyFlags	properties{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
	VkMemoryRequirements	memRequirements{ 0 };
	VkBufferCreateFlags     createFlags{ 0 };
	VkMemoryAllocateFlags   memoryCreateFlags{ 0 };

	//Advanced
	nve::nvePagedBinding* pageBindings{};

public:
	ComputeBufferInfo(int stride = 0, int length = 0, int binding = 0, bool requires_exact_device_match = true, ComputeBufferType type = ComputeBufferType::Static)
		: stride(stride)
		, length(length)
		, binding(binding)
		, bufferSize(static_cast<unsigned long long> (stride)* length)
		, requires_exact_device_match(requires_exact_device_match)
		, type(type)
	{
		if (type == ComputeBufferType::Sparse)
			createFlags |= VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
	}

	ComputeBufferInfo& operator=(const ComputeBufferInfo& rhs) {
		memcpy_s(this, sizeof(ComputeBufferInfo), &rhs, sizeof(ComputeBufferInfo));
		bufferSize = stride * length;
	}
};

struct ComputeBuffer : public DescriptiveObject {
	ComputeBuffer(const ComputeBufferInfo& requested);
	~ComputeBuffer();

	void Deallocate();
	static VkBufferCreateInfo BufferInfo(const ComputeBufferInfo& computebufferinfo);

	VkMemoryRequirements MemoryRequirements();


private:
	void createDescriptorPool();
	void createDescriptorSet();
	void createDescriptorLayout();
	bool createBuffer(ComputeBufferInfo& computebufferinfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory);


public:
	ComputeBufferInfo info;

	VkBuffer buffer{};
	VkDeviceMemory memory{};
	int bindPoint;

	void setBufferData(const void* dataArray, VkDeviceSize offsetDst = 0, VkDeviceSize copyLength = 0, nve::ProductionPackage* context = nullptr, VkFence* fence = nullptr);
	void readBufferData(void* dataArray, VkDeviceSize offset = 0, VkDeviceSize CopyLength = 0, nve::ProductionPackage* context = nullptr);
	void Clear();

	/// <summary>
	///  creates a basic descriptor set of storage buffers
	/// </summary>
	/// <param name="count"></param>
	static std::vector<VkDescriptorSetLayoutBinding> generateBasicDescriptorSet(std::vector<DescriptorPosition> indexes, uint32_t flag = CommonStages) {
		std::vector< VkDescriptorSetLayoutBinding> bindings;

		for (size_t i = 0; i < indexes.size(); i++)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = 0;
			layoutBinding.descriptorCount = indexes[i].maxCount;

			layoutBinding.descriptorType = indexes[i].type;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = flag;

			bindings.push_back(layoutBinding);
		}

		return bindings;
	}
};