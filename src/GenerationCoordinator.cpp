#include "GenerationCoordinator.h"
#include "ChunkInterface.h"
#include "ChunkConst.h"
#include "RenderPassGenerator.h"
#include "../shaders/IndirectWorkGroups.h"
#include "ManualRenderPass.h"
#include "../shaders/ChunkGenerationConst.h"
GenerationCoordinator::GenerationCoordinator() {



   
    int length = 83333;
    context = new nve::ProductionPackage(QueueType::Graphics);
    UnexpandedNodeA = new ComputeBuffer(ComputeBufferInfo(sizeof(nve::GPUMemory::UnexpandedNode), length));
    UnexpandedNodeB = new ComputeBuffer(ComputeBufferInfo(sizeof(nve::GPUMemory::UnexpandedNode), length));

    auto info = ComputeBufferInfo(sizeof(IndirectWorkGroups), 1);
    info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    IndirectBuffer  = new ComputeBuffer(info);

    // Heightmap
    int heightmapLength = heightmap_dimensions.x * heightmap_dimensions.y;
    HeightmapData = new ComputeBuffer(ComputeBufferInfo(sizeof(float), heightmapLength));
}

void GenerationCoordinator::BuildChunkTest(uint32_t volumeID, ChunkID chunk_pos) {
    nve::Chunks::TEMP_ALLOCATOR_CONST temp{};
    temp.volumeID = volumeID;
    temp.ChunkPosition = chunk_pos;
    temp.quality = MaxQuality;


    std::vector< nve::Chunks::TEMP_ALLOCATOR_CONST> temp_const{};


    nve::ManualRenderPass pass = nve::ManualRenderPass(context);

    // Create padded heightmap data
    pass.InlineShaderDispatch(Shaders::GenerateHeightMap, ceil(heightmap_dimensions.x / 8.0), ceil(heightmap_dimensions.y / 8.0), 1, sizeof(chunk_pos), &chunk_pos, MonoidList(1).bind(HeightmapData)->render());

    // Insert barrier before using data
    pass.InsertMemoryBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);



    MonoidList allocA = MonoidList(5);
    ChunkInterface::instance->BindChunkMemory( //1
        ChunkInterface::instance->BindChunkHeader(&allocA)) //0

        ->bind(UnexpandedNodeA) // 2
        ->bind(UnexpandedNodeB) // 3
        ->bind(HeightmapData);  // 4

    MonoidList allocB = MonoidList(5);
    ChunkInterface::instance->BindChunkMemory( //1
        ChunkInterface::instance->BindChunkHeader(&allocB)) //0

        ->bind(UnexpandedNodeB) // 2
        ->bind(UnexpandedNodeA) // 3
        ->bind(HeightmapData); // 4


    int brickIndirectPtr = 0;

    pass.InlineShaderDispatch(Shaders::VoxelBrickMemoryAllocator, 1, 1, 1, sizeof(nve::Chunks::TEMP_ALLOCATOR_CONST), &temp, allocA.render());

    for (size_t i = 1; i <= MaxQuality; i++)
    {

        // Barrier Compute -> Compute
        pass.InsertMemoryBarrier();

        pass.InlineShaderDispatch(Shaders::CopyListToVoxelBrickInDirect, 1, 1, 1, sizeof(int), &brickIndirectPtr,
            MonoidList(2)
            .bind(((temp.quality & 1) ? UnexpandedNodeA : UnexpandedNodeB))
            ->bind(IndirectBuffer)
            ->bind(((temp.quality & 1) ? UnexpandedNodeB : UnexpandedNodeA))
            ->render());
        // Barrier Compute -> Indirect Read
        pass.InsertMemoryBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);

        temp.quality = MaxQuality - i;
        pass.InlineShaderIndirect(Shaders::VoxelBrickMemoryAllocator, IndirectBuffer, 0, sizeof(nve::Chunks::TEMP_ALLOCATOR_CONST), &temp, ((temp.quality & 1) ? allocB : allocA).render());
    }
  
   
    pass.execute();





    //int nodesToCheck = 100;

    //nve::GPUMemory::UnexpandedNode* data = (nve::GPUMemory::UnexpandedNode*)malloc(sizeof(nve::GPUMemory::UnexpandedNode) * nodesToCheck);

    //UnexpandedNodeB->readBufferData(data, 0, sizeof(nve::GPUMemory::UnexpandedNode) * nodesToCheck);
    //std::vector< nve::GPUMemory::UnexpandedNode> mem{};
    //for (size_t i = 0; i < nodesToCheck; i++)
    //{
    //    mem.push_back(data[i]);
    //}

    //IndirectWorkGroups* idata = (IndirectWorkGroups*)malloc(sizeof(IndirectWorkGroups) * 1);

    //IndirectBuffer->readBufferData(idata, 0, sizeof(IndirectWorkGroups) * 1);
    //std::vector< IndirectWorkGroups> imem{};
    //for (size_t i = 0; i < 1; i++)
    //{
    //    imem.push_back(idata[i]);
    //}


 //   ChunkInterface::instance->CheckGPUMemory();

}
