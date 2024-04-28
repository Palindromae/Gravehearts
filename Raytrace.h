#pragma once

#include <glm/glm.hpp>
#include "nvvk/descriptorsets_vk.hpp"
#include "nvvk/raytraceKHR_vk.hpp"
#include "nvvk/sbtwrapper_vk.hpp"
#include "ObjStruct.h"

#include "shaders/host_device.h"
#include "src/Tlas.h"

extern std::vector<std::string> defaultSearchPaths;

class Raytrace
{
public:
  void setup(const VkDevice& device, const VkPhysicalDevice& physicalDevice, nvvk::ResourceAllocator* allocator, uint32_t queueFamily);
  void destroy();

  void updateRtDescriptorSet(const VkImageView& outputImage);

  void updateRtDescriptorSetPerFrame();

  void createRaytracePipeline(VkDescriptorSetLayout& sceneDescLayout);

  void createRtDescriptorSet(const VkImageView& outputImage);

  void raytrace(const VkCommandBuffer& cmdBuf, const glm::vec4& clearColor, VkDescriptorSet& sceneDescSet, VkExtent2D& size, PushConstantRaster& sceneConstants);

  VkAccelerationStructureBuildSizesInfoKHR CalculateMemoryRequirements();

  void createImplictBuffers();

  auto implicitToVkGeometryKHR();

  auto objectToVkGeometryKHR(const ObjModel& model);

  void BuildChunkBLAS();
  void createBottomLevelAS(std::vector<ObjModel>& models);
  void createTopLevelAS(std::vector<ObjInstance>& instances);
  void BuildTLAS();
  nvvk::Buffer pos_buffer;

	private:

		  nvvk::ResourceAllocator* m_alloc{nullptr};  // Allocator for buffer, images, acceleration structures
  VkPhysicalDevice         m_physicalDevice;
  VkDevice                 m_device;
  int                      m_graphicsQueueIndex{0};
  nvvk::DebugUtil          m_debug;  // Utility to name objects
  nvvk::SBTWrapper         m_sbtWrapper;


  VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
  nvvk::RaytracingBuilderKHR                        m_rtBuilder;
  nvvk::DescriptorSetBindings                       m_rtDescSetLayoutBind;
  VkDescriptorPool                                  m_rtDescPool;
  VkDescriptorSetLayout                             m_rtDescSetLayout;
  VkDescriptorSet                                   m_rtDescSet;
  std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;
  VkPipelineLayout                                  m_rtPipelineLayout;
  VkPipeline                                        m_rtPipeline;
  nvvk::Buffer                                      m_rtSBTBuffer;

  nvvk::Buffer               BlasBuffer;
  nvvk::Buffer               TlasBuffer;
  VkAccelerationStructureKHR TLAS;
  VkAccelerationStructureKHR BLAS;

    PushConstantRay m_pcRay{};
public:
	Tlas Tlas_obj{};
};