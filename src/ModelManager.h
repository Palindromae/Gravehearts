#pragma once
#include <vector>
#include "Model.h"
#include "ComputeBuffer.h"
#include <nvvk/vulkanhppsupport.hpp>

class ModelManager {
	nve::ProductionPackage* context{};
	nvvk::Buffer               BlasBuffer;
	std::vector<Model> StaticModels{};
	std::vector<VkDeviceAddress> BlasPtr{};
	//VkDeviceAddress BlasBufferAddress;
	std::vector<VkAccelerationStructureKHR> BlasModels{};

public:
	ComputeBuffer* instanceBuffer{};
	inline static ModelManager* instance{};
	ModelManager();

	VkDeviceAddress GetModelBlasPtr(int model);

	void BuildModelBLAS();
};