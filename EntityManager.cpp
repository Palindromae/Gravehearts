#include "EntityManager.h"
#include "src/ModelManager.h"

EntityManager::EntityManager(int MaxEntities) : EntityCount(MaxEntities) {

	if (instance != nullptr)
		throw std::exception("Cannot have more than one entity managers!");
	instance = this;

	EntityData = (VkAccelerationStructureInstanceKHR*)malloc(sizeof(VkAccelerationStructureInstanceKHR) * MaxEntities);
	EntityGPUData = (EntityGPUStruct*)malloc(sizeof(EntityGPUStruct) * MaxEntities);

	VkAccelerationStructureInstanceKHR null_instance{};
	null_instance.flags = 0x00;
	null_instance.mask = 0x00;
	null_instance.instanceCustomIndex = 0;
	null_instance.instanceShaderBindingTableRecordOffset = 0;
	null_instance.transform = {};
	null_instance.accelerationStructureReference = ModelManager::instance->GetModelBlasPtr(0);

	for (size_t i = 0; i < MaxEntities; i++)
	{
		EntityData[i] = null_instance;
	}


	context = new nve::ProductionPackage(QueueType::Graphics);

	EntityTlas.setup();
	EntityTlas.SetupInstanceBuffer(MaxEntities);
	EntityInstance_Buffer = EntityTlas.GetInstanceBuffer();

	EntityData_Buffer = new ComputeBuffer(ComputeBufferInfo(sizeof(EntityGPUStruct), MaxEntities));
}


VkDeviceAddress EntityManager::GetModelBlasPtr(int model) {
	return ModelManager::instance->GetModelBlasPtr(model);
}

// Needs to take a command buffer

void EntityManager::UpdateBuffer() {
	EntityInstance_Buffer->setBufferData(EntityData,    0, EntityCount, context, &context->fence);
	EntityData_Buffer->    setBufferData(EntityGPUData, 0, EntityCount, context, &context->fence);
}

void EntityManager::BuildTlas()
{
	EntityTlas.BuildTLAS();
}

VkAccelerationStructureInstanceKHR* EntityManager::GetID(int& no) {
	if (nextPositionInArray.size() > 0)
	{
		
		VkAccelerationStructureInstanceKHR* counter = nextPositionInArray.front();
		int no = (EntityData - counter) / sizeof(VkAccelerationStructureInstanceKHR);
		nextPositionInArray.pop();

		return counter;
	}
	no = maxPos;
	return &EntityData[maxPos++];
}
EntityGPUStruct* EntityManager::GetDataPtr()
{
	if (nextEntityGPUPos.size() > 0)
	{
		EntityGPUStruct* counter = nextEntityGPUPos.front();
		nextEntityGPUPos.pop();

		return counter;
	}
	return &EntityGPUData[maxPosData++];
}

void EntityManager::ReturnID(VkAccelerationStructureInstanceKHR* id) {
	nextPositionInArray.push(id);
}

void EntityManager::ReturnGPUData(EntityGPUStruct* data)
{
	nextEntityGPUPos.push(data);
}

EntityManager::~EntityManager() {
	free(EntityData);
}
