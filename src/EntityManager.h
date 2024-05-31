#pragma once
#include "ComputeBuffer.h"
#include "ProductionPackage.h"
#include "Tlas.h"
#include "EntityGPUStruct.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <vulkan/vulkan_core.h>
#include <corecrt_malloc.h>
#include <exception>
class EntityManager {

	int EntityCount;
	VkAccelerationStructureInstanceKHR* EntityData{};
	//EntityGPUStruct* EntityGPUData{};
	
	std::queue<VkAccelerationStructureInstanceKHR*> nextPositionInArray{};
	std::queue<EntityGPUStruct*> nextEntityGPUPos{};
	int maxPos{};
	int maxPosData{};
	nve::ProductionPackage* context{};
	ComputeBuffer* EntityInstance_Buffer{};

////// Entity Spacial Data /////////////////////
	glm::vec3* PositionData;
	glm::quat* RotationData;
	int* ModelData;
///// 
public:
	static inline EntityManager* instance{};
	ComputeBuffer* EntityData_Buffer{};

	Tlas EntityTlas{};

	EntityManager(int MaxEntities);

	VkDeviceAddress GetModelBlasPtr(int model);

	// Needs to take a command buffer
	void UpdateBuffer();

	void BuildTlas();


	VkAccelerationStructureInstanceKHR* GetID(int& no);

	EntityGPUStruct* GetDataPtr();

	void GetEntityArr(glm::vec3*& PositionData, glm::quat*& RotationData);

	void ReturnID(VkAccelerationStructureInstanceKHR* id);
	void ReturnGPUData(EntityGPUStruct* data);

	~EntityManager();

};