#include "ModelManager.h"
#include "ManualRenderPass.h"

ModelManager::ModelManager() {


	if (instance != nullptr) {
		throw std::exception("cant have more than one model manager");
	}

	instance = this;
	context = new nve::ProductionPackage(QueueType::Graphics);

	// Load in Models from disk

	// Ignoring previous step for now
	StaticModels.push_back(Model(glm::vec3(0)));
	StaticModels.push_back(Model(glm::vec3(10)));

	int dataPtr = 0;
	for (size_t i = 0; i < StaticModels.size(); i++)
	{
		StaticModels[i].DataPtr = dataPtr;
		dataPtr += 0; // Example for future when dataptr becomes nessary
	}

	auto buffer_info = ComputeBufferInfo(sizeof(Model), StaticModels.size());
	buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	buffer_info.memoryCreateFlags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	modelBuffer = new ComputeBuffer(buffer_info);

	modelBuffer->setBufferData(StaticModels.data(), 0, StaticModels.size(), context, &context->fence);

	BuildModelBLAS();
}


VkDeviceAddress ModelManager::GetModelBlasPtr(int model) {
	return BlasPtr[model];
}

int SetSize(int size) {
	return std::ceil(size / 256.0) * 256;
}

void ModelManager::BuildModelBLAS()
{

	VkDeviceSize ScratchBuildSize = 0;
	int BufferSize = 0;

	std::vector< VkAccelerationStructureBuildGeometryInfoKHR> BLASCreateInfos{};
	std::vector< VkDeviceSize> BLASSize{};
	BlasModels.reserve(StaticModels.size());
	for (size_t i = 0; i < StaticModels.size(); i++)
	{
		VkBufferDeviceAddressInfo b_info{};
		b_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		b_info.buffer = modelBuffer->buffer;
		VkAccelerationStructureGeometryAabbsDataKHR aabbsData{};
		aabbsData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
		aabbsData.data.deviceAddress = CommandDispatcher->GetBufferDeviceAddress(b_info);
		aabbsData.stride = sizeof(Model);


		VkAccelerationStructureGeometryKHR geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
		geometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;  // or other flags as needed
		geometry.geometry.aabbs = aabbsData;

		VkAccelerationStructureBuildGeometryInfoKHR blasCreateInfo = {};
		blasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		blasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		blasCreateInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		blasCreateInfo.geometryCount = 1;          // Number of geometries (AABBs) to include
		blasCreateInfo.pGeometries = &geometry;  // Pointer to the geometry array
		blasCreateInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		blasCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

		VkAccelerationStructureBuildSizesInfoKHR info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		CommandDispatcher->GetAccelerationStructureInfo(&blasCreateInfo, 1, info);
		ScratchBuildSize = std::max(info.buildScratchSize, ScratchBuildSize);


		BufferSize += SetSize(info.accelerationStructureSize);

		BLASCreateInfos.push_back(blasCreateInfo);
		BLASSize.push_back(info.accelerationStructureSize);

	}

	auto ScrapBuffer = CommandDispatcher->m_alloc->createBuffer(ScratchBuildSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	BlasBuffer = CommandDispatcher->m_alloc->createBuffer(BufferSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	VkBufferDeviceAddressInfo BlasBuffer_getinfo{};
	BlasBuffer_getinfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	BlasBuffer_getinfo.buffer = BlasBuffer.buffer;
	//BlasBufferAddress = CommandDispatcher->GetBufferDeviceAddress(BlasBuffer_getinfo);

	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRanges{};
	VkAccelerationStructureBuildRangeInfoKHR ranges{};
	ranges.primitiveCount = 1;

	int offset = 0;
	for (size_t i = 0; i < StaticModels.size(); i++) // THis is probably WRONG
	{
		VkAccelerationStructureCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		createInfo.size = BLASSize[i];  // Initial size, will be determined by the build operation
		createInfo.offset = offset;
		createInfo.buffer = BlasBuffer.buffer;

		offset += SetSize(createInfo.size);

		VkAccelerationStructureKHR ModelBlas{};
		CommandDispatcher->CreateAccelerationStructure(&createInfo, ModelBlas);
		BlasModels.push_back(ModelBlas);

		BLASCreateInfos[i].dstAccelerationStructure = ModelBlas;


		VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, ScrapBuffer.buffer };
		BLASCreateInfos[i].scratchData.deviceAddress = CommandDispatcher->GetBufferDeviceAddress(info);

		ranges.primitiveOffset = sizeof(Model) * i;
		buildRanges.push_back(new VkAccelerationStructureBuildRangeInfoKHR(ranges));

		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		addressInfo.accelerationStructure = ModelBlas;
		BlasPtr.push_back(CommandDispatcher->GetAccelerationDeviceAddress(addressInfo));

	}


	nve::ManualRenderPass renderpass = nve::ManualRenderPass(context);

	//auto cmd = CommandDispatcher->beginSingleTimeCommands(context);

	for (size_t i = 0; i < StaticModels.size(); i++)
	//for (int i = StaticModels.size() - 1; i >= 0; i--)
	{
		//int i = 1;
		CommandDispatcher->Handle_vkCmdBuildAccelerationStructuresKHR(*renderpass.GetCommandBuffer(), 1, &BLASCreateInfos[i], &buildRanges[i]);

		if(i+1 != StaticModels.size())
			renderpass.InsertMemoryBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);
	}
	renderpass.execute();

	CommandDispatcher->m_alloc->finalizeAndReleaseStaging();

	for (size_t i = 0; i < buildRanges.size(); i++)
	{
		delete buildRanges[i];
	}
}
