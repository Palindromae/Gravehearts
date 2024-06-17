#include "stdafx.h"
#include "LoadComputeShaders.h"
#include "ChunkConst.h"
namespace Shaders{
	const ComputeShader** Shaders;
	constexpr auto ComputeShaderPath = "spv";

	LoadComputeShaders::LoadComputeShaders(VkDevice* device) {

		Shaders = const_cast<const ComputeShader**>(new ComputeShader * [END]); //THIS IS UNSAFE DO NOT EDIT FURTHER

		// Generic Desciptions
		auto ImageBuffer = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 } });
		auto SingleImage = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 } });
		auto ImageImage = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 } });
		auto SamplerImage = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 } });
		auto SamplerImageBuffer = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 },  {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto SamplerImageImage = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 } });
		auto SingleBuffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto BufferBuffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto Buffer3 = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto BufferBufferSparseImage = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages } });
		auto SparseImage = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages} });
		auto SparseImageBuffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto SparseImageBufferBuffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto SparseSparseBufferBuffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages},{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto SparseSparseBuffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages},{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });

		auto Buffer5 = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto Buffer6 = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
		auto Buffer4Image1Buffer = ComputeBuffer::generateBasicDescriptorSet({ {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0},{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });


	

		// Unique Descriptions
		auto ObjectWorldDetection = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 } , { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 } , { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 } , { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,1 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 } });
		auto CopyWorldToBuffer    = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 },{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,2 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,3 },{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,4 } });
		auto WriteUpdatesToBuffer = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 },{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1 },{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,2 } });
		auto AverageImage         = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 } });
		auto DepthOfField		  = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 },  { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 } }); // { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }
		auto DepthOfFieldStart    = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 } });
		auto VoxelOctree          = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages} });
		auto InitialScan          = ComputeBuffer::generateBasicDescriptorSet({ { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,0, nve::Chunks::MaxVariableMipsForSparseImages}, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,0 }, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0},{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0},{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0} });
	///	loadShader("CopyWorldToBuffer.comp.spv", Shaders::CopyWorldToBuffer, sizeof(CopyWorldToBufferConstants), CopyWorldToBuffer, device);

		// Tools
		/*loadShader("Compact1024.comp.spv",			     Shaders::Compact1024,			   sizeof(Compact1024Constants),        SingleBuffer,			  device);
		loadShader("Scan1024.comp.spv",				     Shaders::Scan1024,				   sizeof(CopyWorldToBufferConstants),  SingleBuffer,			  device);
		loadShader("VoxelCompression.comp.spv",			 Shaders::VoxelCompression,		   sizeof(CompressVoxelConstants),	    SingleBuffer,	     	  device);
		loadShader("AttachKeysToCompression.comp.spv",	 Shaders::AttachKeysToCompression, sizeof(AttachKeysToVoxelsConstants), SingleBuffer,             device);
		loadShader("WriteUpdatesToBuffer.comp.spv",		 Shaders::WriteUpdatesToBuffer,	   sizeof(WriteUpdatesToBufferConst),   WriteUpdatesToBuffer,	  device);
		loadShader("AddTexture.comp.spv",				 Shaders::AddTexture,			   sizeof(TextureAddConstants),		    SamplerImage,			  device);
		loadShader("Threshold.comp.spv",				 Shaders::Threshold,			   sizeof(ThresholdConstants),          SamplerImage,			  device);
		loadShader("AverageImage.comp.spv",				 Shaders::AverageImage,			   sizeof(AverageImageConstants),       AverageImage,			  device);
		loadShader("CircularBokeh.comp.spv",			 Shaders::CircularBokeh,		   sizeof(CircularBokehConstants),      SamplerImage,			  device);
		loadShader("Downsample13.comp.spv",				 Shaders::Downsample13,			   sizeof(int),						    SamplerImage,			  device);
		loadShader("Upsample3x3.comp.spv",				 Shaders::Upsample3x3,			   sizeof(int),						    SamplerImage,			  device);
		loadShader("MultiplyImageByConstant.comp.spv",	 Shaders::MultiplyByConstant,	   sizeof(float),					    SamplerImage,			  device);
		loadShader("XFloatCompress.comp.spv",			 Shaders::XFloatCompress,		   sizeof(XFloatCompressConstants),	    SingleBuffer,			  device);
		loadShader("MaxFilter.comp.spv",			     Shaders::MaxFilter,		       sizeof(MaxFilterConstants),          SamplerImage,			  device);*/

		// Post Processing
		//loadShader("BloomReconstitute.comp.spv",	Shaders::BloomReconstitute,    sizeof(BloomReconstituteConstants),	 SamplerImageImage, device);
		//loadShader("Vignette.comp.spv",				Shaders::Vignette,			   sizeof(VignetteConstants),		     SamplerImage,		device);
		//loadShader("WhiteBalance.comp.spv",			Shaders::WhiteBalance,		   sizeof(WhiteBalanceConstants),		 SamplerImage,		device);
		//loadShader("ChromaticAberration.comp.spv",	Shaders::ChromaticAberration,  sizeof(ChromaticAberrationConstants), SamplerImage,		device);
		//loadShader("DepthOfField.comp.spv",			Shaders::DepthOfField,		   sizeof(DofConstants),		         DepthOfField,		device);
		//loadShader("DofInitialize.comp.spv",		Shaders::DofStart,		       sizeof(DofConstants),				 DepthOfFieldStart,	device);
		//loadShader("Exposure.comp.spv",		     	Shaders::Exposure,		       sizeof(ExposureConstants),            SamplerImageBuffer,device);


		// Tonemapper
		/*loadShader("HillAces.comp.spv",			 Shaders::HillAces,			sizeof(TonemappingConstants), SamplerImage,		  device);
		loadShader("Reinhard.comp.spv",			 Shaders::Reinhard,			sizeof(TonemappingConstants), SamplerImage,		  device);
		loadShader("ReinhardExtended.comp.spv",  Shaders::ReinhardExtended, sizeof(TonemappingConstants), SamplerImageBuffer, device);
		loadShader("Shlick.comp.spv",			 Shaders::Schlick,			sizeof(TonemappingConstants), SamplerImageBuffer, device);
		loadShader("TumblinRushmeier.comp.spv",  Shaders::TumblinRushmeier, sizeof(TonemappingConstants), SamplerImageBuffer, device);
		loadShader("JodieReinhard.comp.spv",     Shaders::JodieReinhard,    sizeof(TonemappingConstants), SamplerImageBuffer, device);
		loadShader("AcesFilmic.comp.spv",        Shaders::AcesFilmic,       sizeof(TonemappingConstants), SamplerImageBuffer, device);*/


		// Rendering
		//loadShader("InitialWorldScan.comp.spv", Shaders::InitialScan, sizeof(InitialScanConstants), InitialScan, device);


		// Chunks
		/*loadShader("UpdateChunkMapping.comp.spv",     Shaders::UpdateChunkMapping,     sizeof(UpdateChunkMappingsConstants),  SingleBuffer,	  	 device);
		loadShader("UpdateChunkMappingBulk.comp.spv", Shaders::UpdateChunkMappingBulk, sizeof(glm::ivec2),                    BufferBuffer,	  	 device);
		loadShader("DownscaleOctree.comp.spv",        Shaders::DownscaleOctree,        sizeof(DownscaleOctreeConstants),      SparseSparseBuffer, device);
		loadShader("VacantChunkGenerator.comp.spv",   Shaders::VacantChunkGenerator,   sizeof(VacantChunkGeneratorConstants), SparseImage,		 device);*/
		loadShader("WriteNewChunkVolume.comp.spv",   Shaders::WriteNewChunkVolume,   sizeof(CreateChunkConst), BufferBuffer,		 device);
		loadShader("UpdateChunkInVolume.comp.spv",   Shaders::UpdateChunkVolume,     sizeof(CreateChunkConst), BufferBuffer,		 device);

		// SVO
	/*	loadShader("ChunkNodeAllocator.comp.spv",		   Shaders::ChunkNodeAllocator,           sizeof(ChunkNodeAllocatorConstants),   Buffer5Image1,       device);
		loadShader("OverworldBrickSetter.comp.spv",		   Shaders::OverworldBrickSetter,         sizeof(OverworldBrickSetterConstants), Buffer4Image1Buffer, device);*/
		loadShader("CopyListToVoxelBrickIndirect.comp.spv", Shaders::CopyListToVoxelBrickInDirect, sizeof(int),                               Buffer3,      device);
		loadShader("VoxelBrickMemoryAllocator.comp.spv",    Shaders::VoxelBrickMemoryAllocator,    sizeof(nve::Chunks::TEMP_ALLOCATOR_CONST), Buffer6,      device);

		// Terrain Generation
		loadShader("GenerateHeightmap.comp.spv", Shaders::GenerateHeightMap, sizeof(glm::ivec3), SingleBuffer, device);

	/*	loadShader("OverworldHeightMap.comp.spv",   Shaders::OverworldHeightMap,    sizeof(OverworldNoiseSetterConstants), BufferBuffer,            device);
		loadShader("SetOverworld3DNoise.comp.spv",  Shaders::SetOverworld3DNoise,   sizeof(Noise3DStruct),				  BufferBuffer,            device);
		loadShader("OverworldBlockSetter.comp.spv", Shaders::OverworldBlockSetter,  sizeof(OverworldBlockSetterConstants), BufferBufferSparseImage, device);
		loadShader("VoxelOctreeAllForOne.comp.spv", Shaders::VoxelOctreeAllForOne,  sizeof(OctreeConstants),				  VoxelOctree,             device);*/
		
		//loadShader("VoxelOctreeAllForOneWithSafetyMips.comp.spv",  Shaders::VoxelOctreeAllForOneWithSafetyMips,  sizeof(OctreeConstants), VoxelOctree, device);

		// Physics
		loadShader("ObjectWorldCollisionDetection.comp.spv", Shaders::ObjectWorldCollisionDetection, sizeof(int), ObjectWorldDetection, device);


		// Utility
		/*loadShader("ReadTexture.comp.spv",         Shaders::ReadTexture,			sizeof(ReadTextureConstants),		SparseImageBuffer,		  device);
		loadShader("ReadChunk.comp.spv",           Shaders::ReadChunk,			sizeof(ReadChunkConstants),			SparseImageBufferBuffer,  device);
		loadShader("ReadChunkInternal.comp.spv",   Shaders::ReadChunkInternal,   sizeof(ReadChunkInternalConstants), SparseSparseBufferBuffer, device);
		loadShader("CopyListToIndirect.comp.spv",  Shaders::CopyListToIndirect,  sizeof(glm::ivec2),					BufferBuffer,			  device);
		loadShader("ZeroOutBuffer.comp.spv",       Shaders::ZeroOutBuffer,       sizeof(int),						SingleBuffer,			  device);
		loadShader("ZeroOutImage.comp.spv",        Shaders::ZeroOutImage,        sizeof(glm::ivec3),				    SingleImage,			  device);*/

	}


	void LoadComputeShaders::loadShader(std::string path, ShaderClass shader, int size, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDevice* device) 
	{
		std::filesystem::path dire = (ComputeShaderPath);
		std::filesystem::path file(path);
		Shaders[shader] = new ComputeShader(*device, bindings, size, (dire / file).string());

	}
}