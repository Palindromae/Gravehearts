#pragma once
#include "ComputeShader.h"
#include "ShaderPushConstants.h"
#include "ComputeBuffer.h"
#include "StoredComputeShaders.h"
namespace Shaders
{
	//WARNING DO NOT MODIFY THIS FURTHER

	struct LoadComputeShaders {


		LoadComputeShaders(VkDevice* device);

		

		LoadComputeShaders();
		void loadShader(std::string path, ShaderClass shader, int size, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDevice* device);
	};


}