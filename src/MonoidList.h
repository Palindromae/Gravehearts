#pragma once
#include <vector>
#include "vulkan/vulkan_core.h"
#include "ComputeBuffer.h"

struct MonoidList {

	std::vector<VkDescriptorSet> setList{};

	MonoidList(int size) {
		setList.reserve(size);
	}

	MonoidList* bind(DescriptiveObject buffer) {
		setList.emplace_back(buffer.set);//make a type that buffer inherits from with descriptor sets 
		return this;
	}
	MonoidList* bind(DescriptiveObject* buffer) {
		setList.emplace_back(buffer->set);//make a type that buffer inherits from with descriptor sets 
		return this;
	}
	MonoidList* bind(VkDescriptorSet* set) {
		setList.emplace_back(*set);//make a type that buffer inherits from with descriptor sets 
		return this;
	}

	MonoidList* bind(VkDescriptorSet set) {
		setList.emplace_back(set);//make a type that buffer inherits from with descriptor sets 
		return this;
	}
	std::vector<VkDescriptorSet> render() {
		return setList;
	}
	std::vector<VkDescriptorSet> complete() {
		return setList;
	}

};