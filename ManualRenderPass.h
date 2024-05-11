#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include "ProductionPackage.h"
#include "nveTypes.h"
namespace nve {

	class RenderPassGenerator {

		std::vector<DeferedShader> subpasses{};
		std::vector<int*> dependancies{}; //in format X + N. Where X is the amount of dependancies for this shader. 0 for [0+] 1 for e.g [1 + 5]
		int index{};
	public:
		RenderPassGenerator(int subpassesCount);
		~RenderPassGenerator();
		int insert(DeferedShader secondary, int dependancy_count = 0, int* dependancies = nullptr);
		void execute(const ProductionPackage* context);
	};

}