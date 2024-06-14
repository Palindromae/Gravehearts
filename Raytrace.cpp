#include "Raytrace.h"
#include "nvvk/shaders_vk.hpp"
#include "nvh/fileoperations.hpp"
#include "nvvk/buffers_vk.hpp"
#include <obj_loader.h>
#include "shaders/AABBDef.h"
#include "src/ChunkInterface.h"
#include "src/EntityManager.h"

void Raytrace::setup(const VkDevice& device, const VkPhysicalDevice& physicalDevice, nvvk::ResourceAllocator* allocator, uint32_t queueFamily)
{
  m_device             = device;
  m_physicalDevice     = physicalDevice;
  m_alloc              = allocator;
  m_graphicsQueueIndex = queueFamily;

  // Requesting ray tracing properties
  VkPhysicalDeviceProperties2 prop2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
  prop2.pNext = &m_rtProperties;
  vkGetPhysicalDeviceProperties2(m_physicalDevice, &prop2);

  m_rtBuilder.setup(m_device, allocator, m_graphicsQueueIndex);
  m_sbtWrapper.setup(device, queueFamily, allocator, m_rtProperties);
  m_debug.setup(device);
}

void Raytrace::destroy() {
  m_sbtWrapper.destroy();
  m_rtBuilder.destroy();
  vkDestroyDescriptorPool(m_device, m_rtDescPool, nullptr);
  vkDestroyDescriptorSetLayout(m_device, m_rtDescSetLayout, nullptr);
  vkDestroyPipeline(m_device, m_rtPipeline, nullptr);
  vkDestroyPipelineLayout(m_device, m_rtPipelineLayout, nullptr);
  m_alloc->destroy(m_rtSBTBuffer);

}

void Raytrace::updateRtDescriptorSet(const VkImageView& outputImage)
{
  VkDescriptorImageInfo imageInfo{{}, outputImage, VK_IMAGE_LAYOUT_GENERAL};
  VkWriteDescriptorSet  wds = m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eOutImage, &imageInfo);

  std::vector<VkWriteDescriptorSet> writes{};
  writes.push_back(wds);
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}

void Raytrace::updateRtDescriptorSetPerFrame()
{
  VkAccelerationStructureKHR tlas = EntityManager::instance->EntityTlas.acceleration_structure;

  VkWriteDescriptorSetAccelerationStructureKHR descASInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
  descASInfo.accelerationStructureCount = 1;
  descASInfo.pAccelerationStructures = &tlas;

  VkAccelerationStructureKHR tlasB = ChunkInterface::instance->GetChunkTlas()->acceleration_structure;
  VkWriteDescriptorSetAccelerationStructureKHR descASInfoB{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
  descASInfoB.accelerationStructureCount = 1;
  descASInfoB.pAccelerationStructures = &tlasB;

  VkWriteDescriptorSet  was = m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eTlasEntities, &descASInfo);
  VkWriteDescriptorSet  twas = m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eTlasChunks, &descASInfoB);

  std::vector<VkWriteDescriptorSet> writes{};
  writes.push_back(was);
  writes.push_back(twas);
  vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
}


VkAccelerationStructureBuildSizesInfoKHR Raytrace::CalculateMemoryRequirements()
{
  std::vector<VkAccelerationStructureGeometryKHR> geometries;

  //for (size_t i = 0; i < ChunkBVH::NoChunksPerBVH; i++)
  //{
  VkAccelerationStructureGeometryKHR geometry = {};
  geometry.sType                              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  geometry.geometryType                       = VK_GEOMETRY_TYPE_AABBS_KHR;
  geometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;  // or other flags as needed

  VkAccelerationStructureGeometryAabbsDataKHR aabbsData{};
  aabbsData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
  //aabbsData.data.hostAddress = &aabb;
  //aabbsData.stride = sizeof(VkAabbPositionsKHR);

  //	geometry.geometry.triangles = {};
  //	geometry.geometry.instances = {};
  geometry.geometry.aabbs = aabbsData;


  geometries.push_back(geometry);


  VkAccelerationStructureBuildGeometryInfoKHR blasCreateInfo = {};
  blasCreateInfo.sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  blasCreateInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
  blasCreateInfo.geometryCount = 1;                  // Number of geometries (AABBs) to include
  blasCreateInfo.pGeometries   = geometries.data();  // Pointer to the geometry array
  blasCreateInfo.ppGeometries  = nullptr;            // Pointer to the geometry array
  blasCreateInfo.flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
  blasCreateInfo.mode          = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

  VkAccelerationStructureBuildSizesInfoKHR buildsizes{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
  const uint32_t                           count = 125;
  vkGetAccelerationStructureBuildSizesKHR(m_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &blasCreateInfo,
                                          &count, &buildsizes);


  return buildsizes;
}

void Raytrace::createImplictBuffers()
{
  using vkBU = VkBufferUsageFlagBits;
  nvvk::CommandPool cmdGen(m_device, m_graphicsQueueIndex);

  // Not allowing empty buffers

  std::vector<AABBDef> poss{};

  AABBDef position{};

  position.minimum = glm::vec3({-10, -10, -10});
  position.maximum = glm::vec3({10, 10, 10});

  poss.push_back(position);

  position.minimum = glm::vec3({10});
  position.maximum = glm::vec3({20});

  poss.push_back(position);

  auto cmdBuf = cmdGen.createCommandBuffer();
  pos_buffer  = m_alloc->createBuffer(cmdBuf, poss,
                                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                          | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);

  cmdGen.submitAndWait(cmdBuf);
  m_alloc->finalizeAndReleaseStaging();

  m_debug.setObjectName(pos_buffer.buffer, "implicitObj");

  // m_debug.setObjectName(m_implObjects.implBuf.buffer, "implicitObj");
  //m_debug.setObjectName(m_implObjects.implMatBuf.buffer, "implicitMat");
}
auto Raytrace::implicitToVkGeometryKHR()
{
  VkDeviceAddress dataAddress = nvvk::getBufferDeviceAddress(m_device, pos_buffer.buffer);

  VkAccelerationStructureGeometryAabbsDataKHR aabbs{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR};
  aabbs.data.deviceAddress = dataAddress;
  aabbs.stride             = sizeof(AABBDef);

  // Setting up the build info of the acceleration
  VkAccelerationStructureGeometryKHR asGeom{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
  asGeom.geometryType   = VK_GEOMETRY_TYPE_AABBS_KHR;
  asGeom.flags          = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;  // For AnyHit
  asGeom.geometry.aabbs = aabbs;


  VkAccelerationStructureBuildRangeInfoKHR offset;
  offset.firstVertex     = 0;
  offset.primitiveCount  = 1;  // Nb aabb
  offset.primitiveOffset = 0;
  offset.transformOffset = 0;

  nvvk::RaytracingBuilderKHR::BlasInput input;
  input.asGeometry.emplace_back(asGeom);
  input.asBuildOffsetInfo.emplace_back(offset);
  return input;
}

auto Raytrace::objectToVkGeometryKHR(const ObjModel& model)
{
  // BLAS builder requires raw device addresses.
  VkDeviceAddress vertexAddress = nvvk::getBufferDeviceAddress(m_device, model.vertexBuffer.buffer);
  VkDeviceAddress indexAddress  = nvvk::getBufferDeviceAddress(m_device, model.indexBuffer.buffer);

  uint32_t maxPrimitiveCount = model.nbIndices / 3;

  // Describe buffer as array of VertexObj.
  VkAccelerationStructureGeometryTrianglesDataKHR triangles{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
  triangles.vertexFormat             = VK_FORMAT_R32G32B32_SFLOAT;  // vec3 vertex position data.
  triangles.vertexData.deviceAddress = vertexAddress;
  triangles.vertexStride             = sizeof(VertexObj);
  // Describe index data (32-bit unsigned int)
  triangles.indexType               = VK_INDEX_TYPE_UINT32;
  triangles.indexData.deviceAddress = indexAddress;
  // Indicate identity transform by setting transformData to null device pointer.
  //triangles.transformData = {};
  triangles.maxVertex = model.nbVertices - 1;

  // Identify the above data as containing opaque triangles.
  VkAccelerationStructureGeometryKHR asGeom{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
  asGeom.geometryType       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
  asGeom.flags              = VK_GEOMETRY_OPAQUE_BIT_KHR;
  asGeom.geometry.triangles = triangles;

  // The entire array will be used to build the BLAS.
  VkAccelerationStructureBuildRangeInfoKHR offset;
  offset.firstVertex     = 0;
  offset.primitiveCount  = maxPrimitiveCount;
  offset.primitiveOffset = 0;
  offset.transformOffset = 0;

  // Our blas is made from only one geometry, but could be made of many geometries
  nvvk::RaytracingBuilderKHR::BlasInput input;
  input.asGeometry.emplace_back(asGeom);
  input.asBuildOffsetInfo.emplace_back(offset);

  return input;
}


void Raytrace::BuildChunkBLAS()
{

  createImplictBuffers();
  nvvk::CommandPool cmdGen(m_device, m_graphicsQueueIndex);

  auto buildSize = CalculateMemoryRequirements();

  auto ScrapBuffer = m_alloc->createBuffer(buildSize.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                                                           | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

  BlasBuffer = m_alloc->createBuffer(buildSize.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
                                                                              | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

  VkAccelerationStructureCreateInfoKHR createInfo = {};
  createInfo.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
  createInfo.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
  createInfo.size   = buildSize.accelerationStructureSize;  // Initial size, will be determined by the build operation
  createInfo.offset = 0;
  createInfo.buffer = BlasBuffer.buffer;

  vkCreateAccelerationStructureKHR(m_device, &createInfo, nullptr, &BLAS);

  int geo_count = 1;


  // TEMP

  // VkAccelerationStructureGeometryAabbsDataKHR aabbsData{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR};
  // aabbsData.data.deviceAddress = dataAddress;
  // aabbsData.stride             = sizeof(ObjImplicit);
  // TEMP

  VkBufferDeviceAddressInfo b_info{};
  b_info.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
  b_info.buffer = pos_buffer.buffer;
  VkAccelerationStructureGeometryAabbsDataKHR aabbsData{};
  aabbsData.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
  aabbsData.data.deviceAddress = vkGetBufferDeviceAddress(m_device, &b_info);
  aabbsData.stride             = sizeof(AABBDef);


  VkAccelerationStructureGeometryKHR geometry = {};
  geometry.sType                              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  geometry.geometryType                       = VK_GEOMETRY_TYPE_AABBS_KHR;
  geometry.flags          = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;  // or other flags as needed
  geometry.geometry.aabbs = aabbsData;

  VkAccelerationStructureBuildGeometryInfoKHR blasCreateInfo = {};
  blasCreateInfo.sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  blasCreateInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
  blasCreateInfo.srcAccelerationStructure = VK_NULL_HANDLE;
  blasCreateInfo.dstAccelerationStructure = BLAS;
  blasCreateInfo.geometryCount            = 1;          // Number of geometries (AABBs) to include
  blasCreateInfo.pGeometries              = &geometry;  // Pointer to the geometry array
  blasCreateInfo.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
  blasCreateInfo.flags                    = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;


  //VkBufferDeviceAddressInfo{.}
  // use global scratch memory or something else then calculate the device address and add on the offset.
  VkBufferDeviceAddressInfo info{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, ScrapBuffer.buffer};
  blasCreateInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(m_device, &info);

  VkAccelerationStructureBuildRangeInfoKHR ranges{};
  ranges.primitiveCount = geo_count;
  ranges.primitiveCount = 2;

  const VkAccelerationStructureBuildRangeInfoKHR* RangeInfos[1] = {&ranges};

  auto cmdBuf = cmdGen.createCommandBuffer();
  vkCmdBuildAccelerationStructuresKHR(cmdBuf, 1, &blasCreateInfo, RangeInfos);

  cmdGen.submitAndWait(cmdBuf);
}

void Raytrace::createBottomLevelAS(std::vector<ObjModel>& models)
{

    
    // BLAS - Storing each primitive in a geometry
  std::vector<nvvk::RaytracingBuilderKHR::BlasInput> allBlas;
  allBlas.reserve(models.size());
  for(const auto& obj : models)
  {
    auto blas = objectToVkGeometryKHR(obj);

    // We could add more geometry in each BLAS, but we add only one for now
    //allBlas.emplace_back(blas);
  }

  auto blas = implicitToVkGeometryKHR();
  allBlas.emplace_back(blas);
  //implicitObj.blasId = static_cast<int>(allBlas.size() - 1);  // remember blas ID for tlas

  m_rtBuilder.buildBlas(allBlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}

void Raytrace::createTopLevelAS(std::vector<ObjInstance>& instances)
{
  std::vector<VkAccelerationStructureInstanceKHR> tlas;
  tlas.reserve(instances.size());
  for(const ObjInstance& inst : instances)
  {
    VkAccelerationStructureInstanceKHR rayInst{};
    rayInst.transform                      = nvvk::toTransformMatrixKHR(inst.transform);  // Position of the instance
    rayInst.instanceCustomIndex            = inst.objIndex;                               // gl_InstanceCustomIndexEXT
    rayInst.accelerationStructureReference = m_rtBuilder.getBlasDeviceAddress(inst.objIndex);
    rayInst.flags                          = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    rayInst.mask                           = 0xFF;       //  Only be hit if rayMask & instance.mask != 0
    rayInst.instanceShaderBindingTableRecordOffset = 0;  // We will use the same hit group for all objects
   // tlas.emplace_back(rayInst);
  }

  VkAccelerationStructureInstanceKHR rayInst{};
  rayInst.transform           = nvvk::toTransformMatrixKHR(glm::mat4(1));  // Position of the instance
  rayInst.instanceCustomIndex = 1;
  rayInst.accelerationStructureReference = m_rtBuilder.getBlasDeviceAddress(static_cast<uint32_t>(0));
  rayInst.flags                          = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
  rayInst.mask                           = 0xFF;       // Only be hit if rayMask & instance.mask != 0
  rayInst.instanceShaderBindingTableRecordOffset = 0;  // We will use the same hit group for all objects (the second one)
  tlas.emplace_back(rayInst);

  m_rtBuilder.buildTlas(tlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}
void Raytrace::BuildTLAS()
{
    Tlas_obj.SetupInstanceBuffer(1);

    int                                instanceCount = 1;
    VkAccelerationStructureInstanceKHR instancesData{};
    instancesData.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    instancesData.mask = 0xFF;

    VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
    addressInfo.accelerationStructure = BLAS;

    instancesData.accelerationStructureReference = CommandDispatcher->GetAccelerationDeviceAddress(addressInfo);
    //instancesData.accelerationStructureReference = m_rtBuilder.getBlasDeviceAddress(static_cast<uint32_t>(implicitObj.blasId));
    instancesData.transform = nvvk::toTransformMatrixKHR(glm::mat4(1));
    instancesData.instanceCustomIndex = 0;
    instancesData.instanceShaderBindingTableRecordOffset = 0;
    std::vector<VkAccelerationStructureInstanceKHR> instances{ instancesData };

    Tlas_obj.setData(instances.data());

    Tlas_obj.BuildTLAS();
  
}


void Raytrace::createRaytracePipeline(VkDescriptorSetLayout& sceneDescLayout)
{
  enum StageIndices
  {
    eRaygen,
    eMiss,
    eClosestHit,
    eIntersect,
    eIntersectChunk,
    eClosestHitChunk,
    eShaderGroupCount
  };

  // All stages
  std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount> stages{};
  VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  stage.pName = "main";  // All the same entry point
  // Raygen
  stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/raytrace.rgen.spv", true, defaultSearchPaths, true));
  stage.stage     = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  stages[eRaygen] = stage;
  // Miss
  stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/raytrace.rmiss.spv", true, defaultSearchPaths, true));
  stage.stage   = VK_SHADER_STAGE_MISS_BIT_KHR;
  stages[eMiss] = stage;
  //// The second miss shader is invoked when a shadow ray misses the geometry. It simply indicates that no occlusion has been found
  //stage.module =
  //    nvvk::createShaderModule(m_device, nvh::loadFile("spv/raytraceShadow.rmiss.spv", true, defaultSearchPaths, true));
  //stage.stage    = VK_SHADER_STAGE_MISS_BIT_KHR;
  //stages[eMiss2] = stage;
  // Hit Group - Closest Hit
  stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/raytrace.rchit.spv", true, defaultSearchPaths, true));
  stage.stage         = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  stages[eClosestHit] = stage;


  // Hit Group - 1
  stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/ChunkTrace.rchit.spv", true, defaultSearchPaths, true));
  stage.stage          = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  stages[eClosestHitChunk] = stage;
  // Hit
  //stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/raytrace2.rahit.spv", true, defaultSearchPaths, true));
  //stage.stage      = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
  //stages[eAnyHit1] = stage;
  // Hit
  stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/raytrace.rint.spv", true, defaultSearchPaths, true));
  stage.stage        = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
  stages[eIntersect] = stage;

  // Hit Group - Closest Hit
  stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/ChunkTrace.rint.spv", true, defaultSearchPaths, true));
  stage.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
  stages[eIntersectChunk] = stage;


  // Call0
  //stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/light_point.rcall.spv", true, defaultSearchPaths, true));
  //stage.stage    = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  //stages[eCall0] = stage;
  //// Call1
  //stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/light_spot.rcall.spv", true, defaultSearchPaths, true));
  //stage.stage    = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  //stages[eCall1] = stage;
  //// Call2
  //stage.module = nvvk::createShaderModule(m_device, nvh::loadFile("spv/light_inf.rcall.spv", true, defaultSearchPaths, true));
  //stage.stage    = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  //stages[eCall2] = stage;


  // Shader groups
  VkRayTracingShaderGroupCreateInfoKHR group{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
  group.anyHitShader       = VK_SHADER_UNUSED_KHR;
  group.closestHitShader   = VK_SHADER_UNUSED_KHR;
  group.generalShader      = VK_SHADER_UNUSED_KHR;
  group.intersectionShader = VK_SHADER_UNUSED_KHR;

  // Raygen
  group.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  group.generalShader = eRaygen;
  m_rtShaderGroups.push_back(group);

  // Miss
  group.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  group.generalShader = eMiss;
  m_rtShaderGroups.push_back(group);

  // Shadow Miss
  //group.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  //group.generalShader = eMiss2;
  //m_rtShaderGroups.push_back(group);


  // closest hit shader
  group.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
  group.generalShader      = VK_SHADER_UNUSED_KHR;
  group.closestHitShader   = eClosestHit;
  group.anyHitShader       = VK_SHADER_UNUSED_KHR;
  group.intersectionShader = eIntersect;
  m_rtShaderGroups.push_back(group);

  // closest hit shader
  group.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
  group.generalShader      = VK_SHADER_UNUSED_KHR;
  group.closestHitShader   = eClosestHitChunk;
  group.anyHitShader       = VK_SHADER_UNUSED_KHR;
  group.intersectionShader = eIntersectChunk;
  m_rtShaderGroups.push_back(group);

  // Callable shaders
  //group.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
  //group.closestHitShader   = VK_SHADER_UNUSED_KHR;
  //group.anyHitShader       = VK_SHADER_UNUSED_KHR;
  //group.intersectionShader = VK_SHADER_UNUSED_KHR;
  //group.generalShader      = eCall0;
  //m_rtShaderGroups.push_back(group);
  //group.generalShader = eCall1;
  //m_rtShaderGroups.push_back(group);
  //group.generalShader = eCall2;
  //m_rtShaderGroups.push_back(group);


  // Push constant: we want to be able to update constants used by the shaders
  VkPushConstantRange pushConstant{VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
                                       | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR,
                                   0, sizeof(PushConstantRay)};


  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstant;

  // Descriptor sets: one specific to ray tracing, and one shared with the rasterization pipeline
  std::vector<VkDescriptorSetLayout> rtDescSetLayouts = {m_rtDescSetLayout, sceneDescLayout};
  pipelineLayoutCreateInfo.setLayoutCount             = static_cast<uint32_t>(rtDescSetLayouts.size());
  pipelineLayoutCreateInfo.pSetLayouts                = rtDescSetLayouts.data();

  vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_rtPipelineLayout);


  // Assemble the shader stages and recursion depth info into the ray tracing pipeline
  VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
  rayPipelineInfo.stageCount = static_cast<uint32_t>(stages.size());  // Stages are shaders
  rayPipelineInfo.pStages    = stages.data();

  rayPipelineInfo.groupCount = static_cast<uint32_t>(m_rtShaderGroups.size());
  rayPipelineInfo.pGroups    = m_rtShaderGroups.data();

  // The ray tracing process can shoot rays from the camera, and a shadow ray can be shot from the
  // hit points of the camera rays, hence a recursion level of 2. This number should be kept as low
  // as possible for performance reasons. Even recursive ray tracing should be flattened into a loop
  // in the ray generation to avoid deep recursion.
  rayPipelineInfo.maxPipelineRayRecursionDepth = 1;  // Ray depth
  rayPipelineInfo.layout                       = m_rtPipelineLayout;

  vkCreateRayTracingPipelinesKHR(m_device, {}, {}, 1, &rayPipelineInfo, nullptr, &m_rtPipeline);

  m_sbtWrapper.create(m_rtPipeline, rayPipelineInfo);
}


void Raytrace::createRtDescriptorSet(const VkImageView& outputImage)
{
  using vkDSLB = VkDescriptorSetLayoutBinding;

  m_rtDescSetLayoutBind.addBinding(RtxBindings::eTlasEntities, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1,
                                   VK_SHADER_STAGE_RAYGEN_BIT_KHR);  // TLAS
  m_rtDescSetLayoutBind.addBinding(RtxBindings::eTlasChunks, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1,
                                   VK_SHADER_STAGE_RAYGEN_BIT_KHR);  // TLAS

  m_rtDescSetLayoutBind.addBinding(RtxBindings::eChunkHeaders, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_INTERSECTION_BIT_KHR);  // Chunk Headers
  m_rtDescSetLayoutBind.addBinding(RtxBindings::eChunkMemory,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_INTERSECTION_BIT_KHR);  // Chunk Memory

  m_rtDescSetLayoutBind.addBinding(RtxBindings::eOutImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);  // Output image

  m_rtDescPool      = m_rtDescSetLayoutBind.createPool(m_device);
  m_rtDescSetLayout = m_rtDescSetLayoutBind.createLayout(m_device);

  VkDescriptorSetAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool     = m_rtDescPool;
  allocateInfo.descriptorSetCount = 1;
  allocateInfo.pSetLayouts        = &m_rtDescSetLayout;
  vkAllocateDescriptorSets(m_device, &allocateInfo, &m_rtDescSet);

  // VkAccelerationStructureKHR tlas = m_rtBuilder.getAccelerationStructure();
  EntityManager::instance->UpdateBuffer();
  EntityManager::instance->BuildTlas();


  VkAccelerationStructureKHR tlas = EntityManager::instance->EntityTlas.acceleration_structure;
  VkWriteDescriptorSetAccelerationStructureKHR descASInfo{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR};
  descASInfo.accelerationStructureCount = 1;
  descASInfo.pAccelerationStructures = &tlas;
  
  VkAccelerationStructureKHR tlasB = ChunkInterface::instance->GetChunkTlas()->acceleration_structure;
  VkWriteDescriptorSetAccelerationStructureKHR descASInfoB{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR};
  descASInfoB.accelerationStructureCount = 1;
  descASInfoB.pAccelerationStructures = &tlasB;



  VkDescriptorImageInfo imageInfo{{}, outputImage, VK_IMAGE_LAYOUT_GENERAL};

  VkDescriptorBufferInfo bufferInfoHeaders{ ChunkInterface::instance->GetChunkHeaders()->buffer,0,VK_WHOLE_SIZE };
  VkDescriptorBufferInfo bufferInfoGPUMemory{ ChunkInterface::instance->GetGPUMemory()->buffer,0,VK_WHOLE_SIZE };

  std::vector<VkWriteDescriptorSet> writes;
  writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eTlasEntities, &descASInfo));
  writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eTlasChunks, &descASInfoB));

  writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eChunkHeaders, &bufferInfoHeaders));
  writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eChunkMemory, &bufferInfoGPUMemory));

  writes.emplace_back(m_rtDescSetLayoutBind.makeWrite(m_rtDescSet, RtxBindings::eOutImage, &imageInfo));
  vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}



//--------------------------------------------------------------------------------------------------
// Ray Tracing the scene
//
void Raytrace::raytrace(const VkCommandBuffer& cmdBuf,
                         const glm::vec4&       clearColor,
                         VkDescriptorSet&       sceneDescSet,
                         VkExtent2D&            size,
                         PushConstantRaster&    sceneConstants)
{
  m_debug.beginLabel(cmdBuf, "Ray trace");
  // Initializing push constant values
  m_pcRay.clearColor           = clearColor;
  m_pcRay.lightPosition        = sceneConstants.lightPosition;
  m_pcRay.lightIntensity       = sceneConstants.lightIntensity;
  //m_pcRay.lightDirection       = sceneConstants.lightDirection;
 // m_pcRay.lightSpotCutoff      = sceneConstants.lightSpotCutoff;
 // m_pcRay.lightSpotOuterCutoff = sceneConstants.lightSpotOuterCutoff;
  m_pcRay.lightType            = sceneConstants.lightType;
 // m_pcRay.frame                = sceneConstants.frame;


  std::vector<VkDescriptorSet> descSets{m_rtDescSet, sceneDescSet};
  vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
  vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipelineLayout, 0,
                          (uint32_t)descSets.size(), descSets.data(), 0, nullptr);
  vkCmdPushConstants(cmdBuf, m_rtPipelineLayout,
                     VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR,
                     0, sizeof(PushConstantRay), &m_pcRay);


  auto& regions = m_sbtWrapper.getRegions();
  vkCmdTraceRaysKHR(cmdBuf, &regions[0], &regions[1], &regions[2], &regions[3], size.width, size.height, 1);

  m_debug.endLabel(cmdBuf);
}