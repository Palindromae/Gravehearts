#pragma once
#include <vulkan/vulkan_core.h>
#include <corecrt_malloc.h>
#include <exception>
#include <queue>
#include "src/ComputeBuffer.h"
#include "src/ProductionPackage.h"
#include "src/Tlas.h"
#include "src/EntityGPUStruct.h"
class EntityManager {

	int EntityCount;
	VkAccelerationStructureInstanceKHR* EntityData{};
	EntityGPUStruct* EntityGPUData{};
	
	std::queue<VkAccelerationStructureInstanceKHR*> nextPositionInArray{};
	std::queue<EntityGPUStruct*> nextEntityGPUPos{};
	int maxPos{};
	int maxPosData{};
	nve::ProductionPackage* context{};
	ComputeBuffer* EntityInstance_Buffer{};
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

	void ReturnID(VkAccelerationStructureInstanceKHR* id);
	void ReturnGPUData(EntityGPUStruct* data);

	~EntityManager();

};