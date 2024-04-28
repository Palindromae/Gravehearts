#pragma once
#include <vulkan/vulkan_core.h>
#include "ComputeBuffer.h"
#include <nvvk/vulkanhppsupport.hpp>
class Tlas {

	ComputeBuffer* instanceBuffer{};
	ComputeBuffer* accelerationBuffer{};

    nvvk::Buffer TlasBuffer;
    nve::ProductionPackage* context{};

public:

    void setup() {
        context = new nve::ProductionPackage(QueueType::Graphics);
    }

    void SetupInstanceBuffer(int instances) {

        if (instanceBuffer != nullptr)
            delete instanceBuffer;

        auto buffer_info = ComputeBufferInfo(sizeof(VkAccelerationStructureInstanceKHR), instances);
        buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        buffer_info.memoryCreateFlags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        instanceBuffer = new ComputeBuffer(buffer_info);

    }

    ComputeBuffer* GetInstanceBuffer() {
        return instanceBuffer;
    }

    void SetupInstanceBuffer(ComputeBuffer*& _instanceBuffer) {
        instanceBuffer = _instanceBuffer;
    }

    void setData(void* data) {
        instanceBuffer->setBufferData(data, 0, instanceBuffer->info.length, context, &context->fence);
    }

    VkAccelerationStructureKHR acceleration_structure{};
    void BuildTLAS()
    {

 


        VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, instanceBuffer->buffer};
        VkAccelerationStructureGeometryInstancesDataKHR instancesVk{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
        instancesVk.data.deviceAddress = CommandDispatcher->GetBufferDeviceAddress(info);

        // Put the above into a VkAccelerationStructureGeometryKHR. We need to put the instances struct in a union and label it as instance data.
        VkAccelerationStructureGeometryKHR topASGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
        topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        topASGeometry.geometry.instances = instancesVk;


        VkAccelerationStructureBuildGeometryInfoKHR TlasGeometryInfo{};
        TlasGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        TlasGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        TlasGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        TlasGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;  // TODO could be changed to fast build if its too slow
        TlasGeometryInfo.geometryCount = 1;
        TlasGeometryInfo.pGeometries = &topASGeometry;
        TlasGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;


        VkAccelerationStructureBuildSizesInfoKHR sinfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
        uint32_t                                 count = instanceBuffer->info.length;

        CommandDispatcher->GetAccelerationStructureInfo(&TlasGeometryInfo, count, sinfo);
  

        TlasBuffer = CommandDispatcher->m_alloc->createBuffer(sinfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

        auto ScrapMemory = CommandDispatcher->m_alloc->createBuffer(sinfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);


        VkBufferDeviceAddressInfo device_info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, ScrapMemory.buffer };
        TlasGeometryInfo.scratchData.deviceAddress = CommandDispatcher->GetBufferDeviceAddress(device_info);

        VkAccelerationStructureCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        createInfo.size = sinfo.accelerationStructureSize;  // Initial size, will be determined by the build operation
        createInfo.offset = 0;
        createInfo.buffer = TlasBuffer.buffer;

        CommandDispatcher->CreateAccelerationStructure(&createInfo, acceleration_structure);


        TlasGeometryInfo.dstAccelerationStructure = acceleration_structure;


        // BUILD
        VkAccelerationStructureBuildRangeInfoKHR ranges{};
        ranges.primitiveCount = count;

        const VkAccelerationStructureBuildRangeInfoKHR* RangeInfos[1] = { &ranges };

        auto cmd = CommandDispatcher->beginSingleTimeCommands(context);

        CommandDispatcher->Handle_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &TlasGeometryInfo, RangeInfos);

        CommandDispatcher->endSingleTimeCommands(context, cmd, true);

        CommandDispatcher->m_alloc->finalizeAndReleaseStaging();
    }
 
};