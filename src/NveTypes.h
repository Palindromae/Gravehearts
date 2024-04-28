#pragma once
#include <functional>
#include <vulkan/vulkan_core.h>

typedef std::function<void(VkCommandBuffer)> DeferedShader;
typedef std::function<VkResult(VkQueue queue)> QueueWork;
