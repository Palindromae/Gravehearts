#pragma once
#include "ComputeBuffer.h"
#include "ProductionPackage.h"
#include "Tlas.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <vulkan/vulkan_core.h>
#include <corecrt_malloc.h>
#include <exception>


class EntityManager {

	VkAccelerationStructureInstanceKHR* EntityData{};
	//EntityGPUStruct* EntityGPUData{};
	
	std::queue<VkAccelerationStructureInstanceKHR*> nextPositionInArray{};
	std::queue<int> nextEntityGPUPos{};
	int maxPos{};
	int maxPosData{};
	nve::ProductionPackage* context{};
	ComputeBuffer* EntityInstance_Buffer{};

////// Unique Entity Data /////////////////////
	int* ModelData;
///// 
public:
	static inline EntityManager* instance{};

	ComputeBuffer* EntityVec3Data;
	ComputeBuffer* EntityVec4Data;
	ComputeBuffer* EntityModel_Buffer{};

	Tlas EntityTlas{};

	EntityManager();

	VkDeviceAddress GetModelBlasPtr(int model);

	// Needs to take a command buffer
	void UpdateBuffer();

	void BuildTlas();

	void RebuildTLS();

	VkAccelerationStructureInstanceKHR* GetID(int& no);

	int GetDataPtr(glm::vec3*& position, glm::quat*& rotation, int*& model);

	void GetEntityArr(ComputeBuffer*& Models);
	void ReturnID(VkAccelerationStructureInstanceKHR* id);
	void ReturnGPUData(int data);
	void SetModel(int id, int Model);
	void SetEntityActive(int id);
	void SetEntityInactive(int id);

	~EntityManager();


};