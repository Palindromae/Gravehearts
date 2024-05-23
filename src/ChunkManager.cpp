#include "ChunkManager.h"
#include "../AABBDef.h"
#include <glm/gtc/matrix_transform.hpp>
#include "ModelManager.h"

ChunkManager::ChunkManager(nve::ProductionPackage* context) : context(context) {
	ChunkVolumes = new ComputeBuffer(ComputeBufferInfo(sizeof(ChunkVolumeGPU), maxVolumes));

	CreateChunkBlas();

	ChunkTlas.setup();
	ChunkTlas.SetupInstanceBuffer(maxVolumes);
}

void ChunkManager::CreateChunkBlas()
{

	AABBDef* chunks = (AABBDef*)malloc(sizeof(AABBDef) * NoChunksPerVolume);
	std::vector<int> indexes{};
	std::vector<AABBDef> datas{};
	for (size_t x = 0; x < NoChunksPerAxii; x++)
	{
		for (size_t y = 0; y < NoChunksPerAxii; y++)
		{
			for (size_t z = 0; z < NoChunksPerAxii; z++)
			{
				int index = ChunkHeaderIndex({ x,y,z });
				chunks[index] = AABBDef({ glm::ivec3( x,y,z)*chunk_dimensions_int,  glm::ivec3(x,y,z) * chunk_dimensions_int + chunk_dimensions_int });
				
				datas.push_back(chunks[index]);
				indexes.push_back(index);
			}
		}
	}


	auto buffer_info = ComputeBufferInfo(sizeof(AABBDef), NoChunksPerVolume);
	buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	buffer_info.memoryCreateFlags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	instanceBuffer = new ComputeBuffer(buffer_info);

	instanceBuffer->setBufferData(chunks, 0, NoChunksPerVolume, context, &context->fence);


	delete[] chunks;


	VkDeviceSize ScratchBuildSize = 0;
	VkDeviceSize accelerationStructureSize = 0;


	VkBufferDeviceAddressInfo b_info{};
	b_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	b_info.buffer = instanceBuffer->buffer;
	VkAccelerationStructureGeometryAabbsDataKHR aabbsData{};
	aabbsData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
	aabbsData.data.deviceAddress = CommandDispatcher->GetBufferDeviceAddress(b_info);
	aabbsData.stride = sizeof(AABBDef);


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
	CommandDispatcher->GetAccelerationStructureInfo(&blasCreateInfo, NoChunksPerVolume, info);
	ScratchBuildSize = info.buildScratchSize;
	accelerationStructureSize = info.accelerationStructureSize ;

	auto ScrapBuffer = CommandDispatcher->m_alloc->createBuffer(ScratchBuildSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	BlasBuffer = CommandDispatcher->m_alloc->createBuffer(accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	VkBufferDeviceAddressInfo BlasBuffer_getinfo{};
	BlasBuffer_getinfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	BlasBuffer_getinfo.buffer = BlasBuffer.buffer;

	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRanges{};

	VkAccelerationStructureCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	createInfo.size = accelerationStructureSize;  // Initial size, will be determined by the build operation
	createInfo.offset = 0;
	createInfo.buffer = BlasBuffer.buffer;

	CommandDispatcher->CreateAccelerationStructure(&createInfo, ChunkBlas);

	blasCreateInfo.dstAccelerationStructure = ChunkBlas;


	VkBufferDeviceAddressInfo infoB{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, ScrapBuffer.buffer };
	blasCreateInfo.scratchData.deviceAddress = CommandDispatcher->GetBufferDeviceAddress(infoB);

	VkAccelerationStructureBuildRangeInfoKHR ranges{};
	ranges.primitiveOffset = 0;
	ranges.primitiveCount = 1;

	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	addressInfo.accelerationStructure = ChunkBlas;

	buildRanges.push_back(&ranges);

	auto cmd = CommandDispatcher->beginSingleTimeCommands(context);

	CommandDispatcher->Handle_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &blasCreateInfo, buildRanges.data());

	CommandDispatcher->endSingleTimeCommands(context, cmd, true);

	CommandDispatcher->m_alloc->finalizeAndReleaseStaging();


	delete instanceBuffer;




	//// Create consts
	VkAccelerationStructureDeviceAddressInfoKHR accel_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	accel_info.accelerationStructure = ChunkBlas;


	createConsts.instance.mask = 0xff;
	createConsts.instance.accelerationStructureReference = CommandDispatcher->GetAccelerationDeviceAddress(accel_info);
	createConsts.instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	createConsts.instance.instanceShaderBindingTableRecordOffset = 1;  // We will use the same hit group for all objects



	nullConsts.instance.mask = 0x00;
	nullConsts.instance.accelerationStructureReference = createConsts.instance.accelerationStructureReference;
	nullConsts.instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	nullConsts.instance.instanceShaderBindingTableRecordOffset = 1;  // We will use the same hit group for all objects
	nullConsts.position.x = 1;
}


ChunkVolume* ChunkManager::CreateChunk(ChunkID position) {

	ChunkID VolumePosition = ChunkVolume::ChunkToVolumeID(position);
	ChunkVolume* volume;
	if (ChunkVolume_Map[VolumePosition] != nullptr)
	{
		volume = ChunkVolume_Map[VolumePosition];
	}
	else {
		// Initiate Volume
		volume = new ChunkVolume(position, GetChunkVolumeID());
		ChunkVolume_Map[VolumePosition] = volume;


		createConsts.position = position;
		createConsts.ptrInMem = volume->StoragePtr;
		createConsts.instance.instanceCustomIndex = volume->StoragePtr;

		glm::mat4 mat = glm::translate(glm::mat4(1), glm::vec3(VolumePosition) * (glm::vec3(NoChunksPerAxii) * chunk_dimensions));
		createConsts.instance.transform = nvvk::toTransformMatrixKHR(mat);

		
		Shaders::Shaders[Shaders::WriteNewChunkVolume]->dispatch(context, 1, 1, 1, sizeof(createConsts), &createConsts, MonoidList(1).bind(ChunkVolumes)->bind(ChunkTlas.GetInstanceBuffer())->render());
		//VkAccelerationStructureInstanceKHR data{};

		//ChunkTlas.GetInstanceBuffer()->readBufferData(&data, 0, 64, context);

		MajorChange = 1;
	}

	return volume;
}

void ChunkManager::DeleteChunk(ChunkID position) {
	ChunkID volumePosition = ChunkVolume::ChunkToVolumeID(position);

	ChunkVolume* volume = ChunkVolume_Map[position];
	volume->DeleteChunk(position);
	nullConsts.position.z = GetChunkVolumeIndexFromPos(volumePosition, position);

	if (volume->ValidateIsVolumeEmpty())
	{
		returnChunkVolumeID(volume->StoragePtr);
		// Volume is empty so delete
		delete volume;
		ChunkVolume_Map.erase(position);

		MajorChange = 1;
		nullConsts.position.y = 1;
	}

	Shaders::Shaders[Shaders::UpdateChunkVolume]->dispatch(context, 1, 1, 1, sizeof(nullConsts), &nullConsts, MonoidList(1).bind(ChunkVolumes)->bind(ChunkTlas.GetInstanceBuffer())->render());
	nullConsts.position.y = 0;
}

void ChunkManager::UpdateGPUStructure() {

	if (MajorChange == 1 ) {
		// Rebuild
		ChunkTlas.BuildTLAS();
	}
}

int ChunkManager::GetChunkVolumeID() {
	if (reusePTR.size() > 0) {
		int ptr = reusePTR.front();
		reusePTR.pop();

		return ptr;
	}

	if (CurrentPtrMax >= maxVolumes)
		throw std::exception("cant have move than the maximum number of chunks");

	return CurrentPtrMax++;
}

void ChunkManager::returnChunkVolumeID(int ID) {
	reusePTR.push(ID);
}

