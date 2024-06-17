#include "EntityManager.h"
#include "ModelManager.h"
#include "../shaders/EntityConst.h"
#include <exception>
#include "CurrentEntityTLSData.h"

EntityManager::EntityManager() {

	if (instance != nullptr)
		throw std::exception("Cannot have more than one entity managers!");
	instance = this;

	EntityData = (VkAccelerationStructureInstanceKHR*)malloc(sizeof(VkAccelerationStructureInstanceKHR) * MaxEntities);
	ModelData = (int*)malloc(sizeof(int) * MaxEntities);

	EntityVec3Data = new ComputeBuffer(ComputeBufferInfo(sizeof(glm::vec3), MaxEntities * EntityVec3::ENDEnitityVec3));
	EntityVec4Data = new ComputeBuffer(ComputeBufferInfo(sizeof(glm::vec4),MaxEntities * EntityVec4::ENDEnitityVec4));
	EntityModel_Buffer = new ComputeBuffer(ComputeBufferInfo(sizeof(int), MaxEntities));

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
	EntityInstance_Buffer->setBufferData(EntityData,    0, MaxEntities, context, &context->fence);
	
	EntityVec3Data->setBufferData(TLS::InterpolatedFrame->PositionBuffer, EntityVec3::Position_Interpolated * MaxEntities, MaxEntities, context);
	EntityVec4Data->setBufferData(TLS::InterpolatedFrame->RotationBuffer, EntityVec4::Rotation_Interpolated * MaxEntities, MaxEntities, context);
	
}

void EntityManager::BuildTlas()
{
	EntityTlas.BuildTLAS();
}

void EntityManager::RebuildTLS() {
	for (size_t i = 0; i < TLS::NumberOfActiveEntities; i++)
	{
		int j = TLS::ActiveEntities[i];

		glm::mat4 mat = glm::translate(glm::mat4(1), TLS::InterpolatedFrame->PositionBuffer[j]);
		mat = mat * glm::toMat4(TLS::InterpolatedFrame->RotationBuffer[j]);
		//mat = mat * glm::scale(glm::mat4(1), scale);
		EntityData[j].transform = nvvk::toTransformMatrixKHR(mat);
	}
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

	position = &TLS::InterpolatedFrame->PositionBuffer[counter];
	rotation = &TLS::InterpolatedFrame->RotationBuffer[counter];
	model = &ModelData[counter];

	return counter;
}

void EntityManager::GetEntityArr(ComputeBuffer*& Models)
{
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
