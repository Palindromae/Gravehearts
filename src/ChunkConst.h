#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <vulkan/vulkan_core.h>
#include "../shaders/ChunkGPUConst.h"
namespace nve::Chunks {

	struct TEMP_ALLOCATOR_CONST {
		ivec3 ChunkPosition;
		int quality;
		int volumeID;
	};

	static const bool CenterChunks = true;

	static const glm::vec3 chunk_scaler = { 1,1,1 };
	
	static const int max_network_updates= 1000;


	static const int quality_restriction = 1;
	
	

	//assumes 16 bit block value
	static const glm::ivec3 texelExtent = { 32,32,32 };
	static const int NoNodesInBuffer     = 125000000;
	static const int MaxAddressableRange = (int)pow(2,30);

	static const int blockIDBits = 9;
	static const int octreeDepthBits = 4;

	static const int OctreeMask = ((int)(pow(2, octreeDepthBits) - 1)) << blockIDBits;
	static int blockBitMask = (int)(pow(2, blockIDBits) - 1);

	static const int LargeChunksOnHorizontalPerLayer =  2;
	static const int LargeChunksOnVerticalPerLayer = 2;

	static const uint32_t MaxVariableMipsForSparseImages = 10;

	static const float RenderDistanceScale = 30; // 60 chunks at 16 block size, reduce to 30 due to size increase
	static const float ActualRenderDistanceScale = 25; //12 is max

	static const int LargeChunksPerLayer = LargeChunksOnHorizontalPerLayer* LargeChunksOnVerticalPerLayer;

	static float greatestResolution = 1 / 8.0f;


	static float WorldSizedTextureMaxResolution = 1;
	static float WorldSizedTextureMinZoom =(float)1.0/ WorldSizedTextureMaxResolution;
	static float HighDetailLowestResolution = 1 / 2.0f;
	static int HighDetailGreatestZoom = 1/ HighDetailLowestResolution;
	static float WorldHighDetailedTextureMaxResolution = greatestResolution;
	static int greatestResolutionBitOffset = -3;
	static int WorldSizeResolutionBitOffset = 0;
	static int TurbulentWorldMips = 1;
	static int mipBoundryBetweenWorldAndDetail = log2(WorldSizedTextureMaxResolution);
	static int maxMipSize = 32;
	static int maxRenderableMipSize = 4;
	static int maxNumberOfWorldMips = log2(maxMipSize / greatestResolution);
	static int MaxRenderDistance = maxMipSize * RenderDistanceScale;

	static int WorldHeight = 256;
	
	static int BrickSize = 8;
	static int BrickImageSize = 1024;
	static int NoChapters = (int)pow(BrickSize,3);

	static int* DistanceToQuality;

	float calculateResolution(int mip);
	float calculateZoom(int mip);
	//Takes in unscaled mip from 0 to X (max sized mip in engine)


	const static VkFormat VoxelImageType = VK_FORMAT_R16_UINT;
		const static int NumberOfBytesInVoxelRepresentation = 2;
	

	struct GPUChunkRequest {
		glm::ivec3 pos;
		glm::uint ID;
	};

	void CreateDistanceToQualityArray();
	

	float calculateGreatestResolutionFromChunkDistance(float distance);
	int maxRenderDistanceFromMipWithNonConstantSize(int mip, int bitOffset);
	int maxRenderDistanceFromSizeWithNonConstantSize(int size, int bitOffset);
	int maxRenderDistanceFromMip(int mip);
	int maxRenderDistanceFromSize(float size);
	int calculateLowestQualityFromChunkDistance(float distance);
	int calculateLowestQualityForChunk(const glm::vec3& chunk_coord_in_chunk_space, const glm::vec3& player_position_chunk_pos);
	int calculateLowestMipFromChunkDistance0Unsafe(float distance);
	int calculateWorldDiameter(int radii, bool centered);
	//Portals
	static int PortalInfluenceBit = 0x01;
	static int PortalLowestMip = 0;
	static int PortalLowestResolution = pow(2, greatestResolutionBitOffset + PortalLowestMip);
	static float portalRenderDistanceScale = 6;
	static int portalIDBits = pow(2, 8)-1;
	static int portalIDOffset = 22;

	float calculateMaxChunkDistanceFromSizePortal(float size);
	float calculateMaxChunkDistanceFromSize(float size);

	float calculateGreatestResolutionFromChunkDistancePortal(float distance);
	int calculateLowestMipFromChunkDistancePortal(float distance);

	glm::ivec3 calculateMaximumRenderDistance(bool center);
	glm::vec3 WorldSpaceToChunkSpace(glm::vec3 worldSpace);
	glm::vec3 CalculateBulkSize(int mip, int NoiseScaleMultiplierMip);

	int CalculateChunkDistance(glm::vec3 vector);
	float CalculateEuclideanChunkDistance(glm::vec3 vector);

	glm::uvec3 alignedDivision(const VkExtent3D& extent, const VkExtent3D& granularity);

	bool IsWorldMip(int mip);

	bool IsHighQuality(int quality);

}
