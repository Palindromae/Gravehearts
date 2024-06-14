#include "EntityManager.h"
#include "ModelManager.h"
#include "../shaders/EntityConst.h"
#include <exception>

EntityManager::EntityManager(int MaxEntities) : EntityCount(MaxEntities) {

	if (instance != nullptr)
		throw std::exception("Cannot have more than one entity managers!");
	instance = this;

	EntityData = (VkAccelerationStructureInstanceKHR*)malloc(sizeof(VkAccelerationStructureInstanceKHR) * MaxEntities);

	EntityVec3Data = new ComputeBuffer(ComputeBufferInfo(sizeof(glm::vec3),EntityCount * EntityVec3::END));
	EntityVec4Data = new ComputeBuffer(ComputeBufferInfo(sizeof(glm::vec4),EntityCount * EntityVec4::END));

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
int EntityManager::GetDataPtr(glm::vec3*& position, glm::quat*& rotation, int*& model)
{
	int counter;
	if (nextEntityGPUPos.size() > 0)
	{
		counter = nextEntityGPUPos.front();
		nextEntityGPUPos.pop();

	} else {

		if (maxPosData == MaxEntities)
			throw new std::exception("Ran out of entities");
	
		counter = maxPosData++;
	}

	position = &PositionData[counter];
	rotation = &rotation[counter];
	model = &model[counter];

	return counter;
}

void EntityManager::GetEntityArr(glm::vec3*& PositionData, glm::quat*& RotationData, ComputeBuffer*& Models)
{
	PositionData = this->PositionData;
	RotationData = this->RotationData;
	Models = this->EntityModel_Buffer;
}



void EntityManager::ReturnID(VkAccelerationStructureInstanceKHR* id) {
	nextPositionInArray.push(id);
}

void EntityManager::ReturnGPUData(int data)
{
	nextEntityGPUPos.push(data);
}

EntityManager::~EntityManager() {
	free(EntityData);
}
