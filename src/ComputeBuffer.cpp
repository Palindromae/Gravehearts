#include "stdafx.h"
#include "ComputeBuffer.h"
#include "SingleTimeCommands.h"
#include "trace.h"
//#include "IDGPUPushRef.h"

ComputeBuffer::ComputeBuffer(const ComputeBufferInfo& requested) {
	memcpy_s(&info, sizeof(info), &requested, sizeof(requested));
	createBuffer(info, buffer, memory);

	if (!(info.usage &= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		return;

	createDescriptorPool();
	createDescriptorLayout();
	createDescriptorSet();
}

ComputeBuffer::~ComputeBuffer()
{
	Deallocate();
}

void ComputeBuffer::Deallocate() {
	DisjointCommandDispatcher::dispatcher->destroyBuffer(buffer, memory, pool, layout);
}

void ComputeBuffer::createDescriptorPool() {
	VkDescriptorPoolSize poolSizes{};

	poolSizes.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSizes;
	poolInfo.maxSets = 1;

	DisjointCommandDispatcher::dispatcher->createDescriptorPool(poolInfo, pool);
}

void ComputeBuffer::createDescriptorLayout() {
	VkDescriptorSetLayoutBinding bufferBinding{};

	bufferBinding.binding = info.binding;
	bufferBinding.descriptorCount = 1;
	bufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferBinding.pImmutableSamplers = nullptr;
	bufferBinding.stageFlags = CommonStages;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &bufferBinding;
	layoutInfo.flags = 0;

	DisjointCommandDispatcher::dispatcher->createDescriptorSetLayout(layoutInfo, layout);
}

void ComputeBuffer::createDescriptorSet() {
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	DisjointCommandDispatcher::dispatcher->createDescriptorSets(allocInfo, set);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = set;
	descriptorWrites.dstBinding = 0;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites.descriptorCount = 1;
	descriptorWrites.pBufferInfo = &bufferInfo;

	DisjointCommandDispatcher::dispatcher->updateDescriptorSets(1, descriptorWrites, 0);
}


VkBufferCreateInfo ComputeBuffer::BufferInfo(const ComputeBufferInfo& computebufferinfo) {
	VkBufferCreateInfo bufferInfo{};

	VkBufferCreateFlags flags = 0;
	bufferInfo.usage = 0;

	if (computebufferinfo.type == ComputeBufferType::Sparse)
		flags |= VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
	else if (computebufferinfo.type == ComputeBufferType::Indirect) {
		bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}
	else if (computebufferinfo.type == ComputeBufferType::Resident)
	{
		flags |= VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
		flags |= VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;

	}

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = computebufferinfo.bufferSize;
	bufferInfo.usage |= computebufferinfo.usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = computebufferinfo.createFlags | flags;


	return bufferInfo;
}

VkMemoryRequirements ComputeBuffer::MemoryRequirements() {
	return DisjointCommandDispatcher::dispatcher->GetBufferMemoryRequirements(this->buffer);
}

void ComputeBuffer::Clear() {
	VkCommandBuffer cmdbuffer = DisjointCommandDispatcher::dispatcher->beginSingleTimeCommands();

	vkCmdFillBuffer(cmdbuffer, buffer, 0, VK_WHOLE_SIZE, 0);
}

bool ComputeBuffer::createBuffer(ComputeBufferInfo& computebufferinfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo{};

	bufferInfo = BufferInfo(computebufferinfo);


	

	DisjointCommandDispatcher::dispatcher->createBuffer(bufferInfo, buffer);

	if (computebufferinfo.type == ComputeBufferType::Sparse) 
	{
		//Require supplied memory
		assert(computebufferinfo.pageBindings != nullptr);

		

		//Bind memory
		VkBindSparseInfo info{};
		info.bufferBindCount = 1;
		
		VkSparseBufferMemoryBindInfo bufferMemoryInfo{};

		bufferMemoryInfo.bindCount = computebufferinfo.bufferSize;
		bufferMemoryInfo.buffer = buffer;



		std::vector<VkSparseMemoryBind > bindings = std::vector<VkSparseMemoryBind>();
		bindings.reserve(computebufferinfo.pageBindings->pageCount);
		long long currentIndex = 0;
		for (int i = 0; i <computebufferinfo.pageBindings->pageCount; i++)
		{
			VkSparseMemoryBind binds{};
			binds.memory = bufferMemory;
			binds.memoryOffset = computebufferinfo.pageBindings->pages[i]->deviceMemloc;
			binds.resourceOffset = currentIndex;
			binds.size = computebufferinfo.pageBindings->pages[i]->size;

			currentIndex += computebufferinfo.pageBindings->pages[i]->size;

			bindings[i] = binds;
			
		}

		bufferMemoryInfo.pBinds = bindings.data();


		info.bufferBindCount = 1;
		info.pBufferBinds = &bufferMemoryInfo;
		

		//info.
		VkFence* fence = DisjointCommandDispatcher::dispatcher->Fences->get();
		DisjointCommandDispatcher::dispatcher->BindSparseMemory(info, *fence);
		DisjointCommandDispatcher::dispatcher->Fences->give(fence);




		return true;
	} 
	if (computebufferinfo.type == ComputeBufferType::Resident)
	{
		return true;
	}
	//Assume type == static
	//Ignore pages even if bound should add in ability for a memory manager to handle static
	return DisjointCommandDispatcher::dispatcher->allocateBufferMemory(buffer, computebufferinfo.properties, computebufferinfo.memoryCreateFlags, bufferMemory, computebufferinfo.requires_exact_device_match);
}

/// <summary>
/// 
/// </summary>
/// <param name="dataArray"></param>
/// <param name="dstOffset">offset in elements</param>
/// <param name="copyLength"> number of element to copy back</param>
void ComputeBuffer::setBufferData(const void* dataArray, VkDeviceSize dstOffset, VkDeviceSize copyLength, nve::ProductionPackage* context, VkFence* fence) {
	void* objData;

	/*
	DisjointCommandDispatcher::dispatcher->copyDataToBuffer(memory, bufferSize, dataArray);

	return;
	*/

	if (copyLength == 0)
		copyLength = info.length;

	

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	ComputeBufferInfo computebufferinfo = info;
	computebufferinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	computebufferinfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	computebufferinfo.requires_exact_device_match = false;
	computebufferinfo.length = std::min(info.bufferSize,copyLength);

	VkDeviceSize copySize = computebufferinfo.length * computebufferinfo.stride;
	computebufferinfo.bufferSize = copySize;

	dstOffset *= computebufferinfo.stride;

	while (!createBuffer(computebufferinfo, stagingBuffer, stagingBufferMemory) && computebufferinfo.length > 4) {
		computebufferinfo.length = ceil(computebufferinfo.length / 2.0);
		computebufferinfo.bufferSize = computebufferinfo.length * computebufferinfo.stride;
	}

	if (buffer == nullptr) {
		computebufferinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		computebufferinfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		createBuffer(computebufferinfo, buffer, memory);
		if (buffer == nullptr) {
			trace_error("Failed to allocate buffer");
			throw std::exception("Failed to allocate buffer");
		}
	}

	if (computebufferinfo.bufferSize == copySize) {
		DisjointCommandDispatcher::dispatcher->setStagingBuffer(stagingBufferMemory, computebufferinfo.bufferSize, dataArray);
		DisjointCommandDispatcher::dispatcher->copyDataToBuffer(stagingBuffer, stagingBufferMemory, buffer, dstOffset, computebufferinfo.bufferSize, context, fence);
	}
	else
	{
		uint32_t offset = 0;
		while (offset + computebufferinfo.bufferSize <= copySize) {
			DisjointCommandDispatcher::dispatcher->setStagingBuffer(stagingBufferMemory, computebufferinfo.bufferSize, (void*)&((const uint8_t*)dataArray)[offset]);
			DisjointCommandDispatcher::dispatcher->copyDataToBuffer(stagingBuffer, stagingBufferMemory, buffer, dstOffset, computebufferinfo.bufferSize, context, fence);
			offset += computebufferinfo.bufferSize;
		}
		if (offset < computebufferinfo.bufferSize) {
			DisjointCommandDispatcher::dispatcher->setStagingBuffer(stagingBufferMemory, computebufferinfo.bufferSize - offset, (void*)&((const uint8_t*)dataArray)[offset]);
			DisjointCommandDispatcher::dispatcher->copyDataToBuffer(stagingBuffer, stagingBufferMemory, buffer, dstOffset, computebufferinfo.bufferSize - offset, context, fence);
		}
	}
}
/// <summary>
///  
/// </summary>
/// <param name="dataArray"></param>
/// <param name="offset"> in bytes </param>
/// <param name="copyLength"> in bytes </param>
void ComputeBuffer::readBufferData(void* dataArray, VkDeviceSize offset, VkDeviceSize copyLength, nve::ProductionPackage* context) {

	if (copyLength == 0)
		copyLength = info.bufferSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	ComputeBufferInfo computebufferinfo = info;
	computebufferinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	computebufferinfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	computebufferinfo.type = ComputeBufferType::Static; //All copying should be done onto a static buffer (for now)
	createBuffer(computebufferinfo, stagingBuffer, stagingBufferMemory);

	DisjointCommandDispatcher::dispatcher->copyDataFromBuffer(stagingBuffer, stagingBufferMemory, buffer, offset, copyLength, dataArray, context);



	
}