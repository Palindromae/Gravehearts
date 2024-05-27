#pragma once
#include <glm/glm.hpp>
#include "vulkan/vulkan_core.h"
//#pragma pack(push,1)

//vec3 must be padded with an extra float
struct OverworldNoiseSetterConstants {
	float overworldHeight;
	float oceanHeight;
	float landHeightAboveOcean;
	float maxHillHeight;
	glm::vec3 scale;
	float PACKING1;
	glm::ivec3 chunkPos;
	float PACKING2;
	glm::ivec3 dimension;
	float PACKING3;
	glm::ivec3 dimensionBase;
	float resolution;
};
struct ReadTextureConstants {
	glm::ivec3 pos;
	int mip;
	int imageHorizontal;
	int imageVertical;
	int imageDepth;
};
struct ReadChunkConstants {
	glm::ivec3 pos;
	int mip;
	int imageHorizontal;
	int imageVertical;
	int imageDepth;
	int chunkIndex;
};

struct ReadChunkInternalConstants {
	glm::ivec3 extent;
	int imageHorizontal;
	int imageVertical;
	int imageDepth;
	int  chunkIndex;
	int DetailMipTotal;
	int MipsUnderOne;
	int mipBoundryBetweenWorldAndDetail;
};

struct OverworldBlockSetterConstants {
	glm::ivec3 chunkPos;
	int Noise3DScale;
	glm::ivec3 chunkDimension;
	float isolevel;
	glm::ivec3 Noise3DDimensions; // size of noise for one chunk
	int Noise2DScale;
	glm::ivec2 Noise2DDimensions;
	int blockSectionSize; // 0 - 15 surface 16 -32 subsurface
	int cavernsStart;
	glm::vec3 ImagePos;
	int mip;
	glm::vec3 extent;
	float resolution;
	glm::ivec3 chunkNoiseId;
	//  float zoom;
	int offsetAmount;


	// Blocks
	int blockBits;
	// Surface Ids
	int surfaceIds; // not sure if this will work
	// Sub-surface Ids 2 1 0 
	int subsurfaceIds;
	// Rock Ids  2 1 0
	int rockIds;

};

struct OverworldBrickSetterConstants {
	glm::ivec3 chunkPos;
	int Noise3DScale;
	glm::ivec3 chunkDimension;
	float isolevel;
	glm::ivec3 Noise3DDimensions; // size of noise for one chunk
	int Noise2DScale;
	glm::ivec2 Noise2DDimensions;
	int blockSectionSize; // 0 - 15 surface 16 -32 subsurface. Size if each block region
	int cavernsStart;
	glm::vec3 ImagePos;
	int mip;
	float resolution;
	glm::ivec3 chunkNoiseId;
	//  float zoom;
	int offsetAmount;

	// Blocks
	int blockBits;
	// Surface Ids
	int surfaceIds; // not sure if this will work
	// Sub-surface Ids 2 1 0 
	int subsurfaceIds;
	// Rock Ids  2 1 0
	int rockIds;

};

struct ChunkNodeAllocatorConstants {
	int quality;
	int maxQuality; // lowest quality (0) is the id of the node that is 1m size with 1/8th size voxels in its bricks
	int chunkID;
	int mipsUnderOne;
	int noiseSize;
};

/*
struct OctreeConstants {
	glm::vec3 TextureSourcePosition;
	int PACKING2;
	glm::vec3 TextureDestinationPosition;
	int PACKING1;
	glm::ivec3 chunkDimensions;
	int PACKING3;
	int mipSource;
	int mipDestination;
	int TextureSource;
	int TextureDestination;
};*/

struct OctreeConstants {
	glm::ivec3 gran;
	int PACKING2;
	glm::vec3 highID;
	int PACKING1;
	glm::ivec3 chunkDimensions;
	int PACKING3;
	glm::vec3 worldID;
	int PACKING4;
	int mipsToCallUp;
	int mipToStartOctree;
	int BlockBitHighest; //all block bits set to 1
	int BlocksBits;      //number of block bits
	int OctreeBitHighest;//all octree bits set to 1

	int DetailMips;
	float dimensionScale;
};

struct CheckIfNullTexel {
	glm::ivec3 chunkPos;
	int mip;
};
struct HeightMapConstants {
	glm::ivec3 chunkPosition;
	int scale;
	glm::ivec3 chunkDimensions;
	int offsetAmount;
	glm::ivec3 chunkRange; // e.g. 1 chunk wide
	float oceanHeight;
	float mapHeight; // what the chunk map says how height this chunk should be relative to the ocean height
	int octaves; 
};
struct Noise3DStruct {
	glm::ivec3 chunkPos;
	int resolution; //how many blocks is represented as one. 1/resolution
	glm::ivec3 dimension;
	int heightResolution; //how many blocks is represented as one.
	glm::ivec3 num_chunks;
	int offsetAmount;

};
/*
struct OctreeConstants {
	glm::ivec3 chunkPos;
	float resolution;
	glm::ivec3 dimension;
	float zoomScale;
	glm::ivec3 dimensionBase;
	int blockIDBits;
	int depthPrev;
	int depthIndex;
	int blockIDByteHighest;
	int imageHorizontal;
	int invImageHorizontal;
};
*/

struct CopyWorldToBufferConstants {
	glm::ivec3 startPosition;
	int mip;
	glm::ivec3 dimensions;
	int leaveAsAir;
	glm::ivec3 copyChunkDimensions; //how many chunks across by how many
	int takeFromAbove;
	glm::vec3 extent;
	int mipBoundryBetweenWorldAndDetail;
	glm::vec3 copyBoundrys;
	int totalMips;
	int BlockBitHighest;
	int MipsUnderOne;

	int VoxelSetIndex;
	int VoxelIndex;
	int MappingIndex;
};

struct PathtracerConstants {
	glm::ivec3 chunkDimensions;
	int blueDimension = 0;
	glm::vec3 extent;
	int NetworkingStart;
	glm::vec3 sunDir;
	int mipBoundryBetweenWorldAndDetail;
	glm::vec3 moonEnlightenedDir;
	int WorldMipTotal;
	int DetailMipTotal;
	int MipsUnderOne; //total number of mips with resolution < 1

	float HorizonsParamG;
	float HorizonsParamA;
	float _Seed;
	int MaxRebounces;

	int BlockBits; //Number of block bits
	int BlockBitHighest; //Block bits all 1
	int OctreeBitHighest;//Octree bits all 1
	float InvDistanceScale;
	float ScanScale;
};

struct Scan1024Constants {
	int amountOfScanning;
	int destinationIndex;
	int CompressionIndex;
	int CounterIndex;
};
struct Compact1024Constants {
	int amountOfScanning;
	int destinationIndex;
	int CompressionIndex;
	int VoxelIndex;
	int CounterIndex;
};
struct CompressVoxelConstants
{
	int bitPattern; ///eg 4 bits per voxel
	int CompressionSetIndex;
	int destinationIndex;
	int VoxelDataIndex;
};
struct AttachKeysToVoxelsConstants {
	glm::ivec3 dimensions;
	int mipLevel;
	int MipsUnderOne;
	int bitPattern; ///eg 4 bits per voxel
	int compressionLength;

	int VoxelDataIndex;
	int CompressionIndex;
	int destinationIndex;
};

struct RaytracerConstants {
	glm::vec3 cameraPos;
	int imageHorizontal;
	glm::ivec3 chunkDimensions;
	int imageHeight;
	glm::vec3 extent;
	int NetworkingStart;
	glm::vec3 sunDir;
	int mipBoundryBetweenWorldAndDetail;
	int mipTotal;
	int WorldMipTotal;
	int DetailMipTotal;
	int MipsUnderOne; //total number of mips with resolution < 1
	float HorizonsParamG;
	float HorizonsParamA;

	int chunkResolutionThresholds[8];//probably should be defined as [8]
};
struct RaytracerBufferConstants {
	glm::mat4 CameraToWorld;
	glm::mat4 InverseProjection;
	glm::mat4 Projection;
	glm::mat4 PastViewMatrix;
	glm::vec4 MoonColour;
	glm::mat4 WorldToMoon;
	float time;         //time from 0 - 1
};

struct GenerateSkyConst
{
	glm::vec3 position;
	int PACKING1;
	glm::vec3 SunDir;
	int PACKING2;
};

struct WriteUpdatesToBufferConst {
	int count;
	int index;
	int sizeofdata;
};

struct AverageImageConstants {
	uint32_t bindingAvg;
	uint32_t highAvg;
	int luminance; // if 1, calcualtes luminance and stores it in R (X)
	float radius = 0; // <= 0 then dont use distance
};

struct TextureAddConstants
{
	glm::vec2 offset;
};

struct ThresholdConstants
{
	float threshold;
};

struct BloomReconstituteConstants {
	int AddOrMix;
	float BlendAmount;
};

struct TonemappingConstants {
	uint32_t Luminance;
	float B;
	float C;
	float InvGamma;
};

struct WhiteBalanceConstants {
	glm::vec3 tint;
	float tintStrength;		   // 0 -> 1
	float temperature;         // 1,000 -> 40,000
	float temperatureStrength; // 0 -> 1
};
struct VignetteConstants {
	float radiiInner; // 0 -> 1
	float radiiOuter; // 0 -> 1
	int shape;
	float strength; // 0 -> 1
};

struct ChromaticAberrationConstants {
	int format; // ChromaticAberrationEnums::format
	float radiiPow;  
	glm::vec2 RedOffset{};
	glm::vec2 GreenOffset{};
	glm::vec2 BlueOffset{};
	float offsetStrength;

};
struct ExposureConstants {
	int shape;
	int type;
	uint32_t avg;
	float target;
	float adaptionSpeed = 1 / (165.0f * 5);
	int initial = 1;
	float MaxExposure = 5;
	float MinExposure = .2;
	float DesaturationCurve = 1.2;
};
struct DofConstants {
	float nearCutOutSize = .005; // for max filter in uv (image size %)

	float maxFarBlur = 1; //customising blur amount probably always 1
	float maxNearBlur = 1; //customising blur amount probably always 1
	float lensAngle =  .5; //for max filtering (*2 + 1)
	float bokehRadius = 1;

	float dt;
	float focalConstant;
	float visualConstant;
	float minNearPlane = .03f;
	float focalRangeAdjustmentSpeed = 8;
};

struct CircularBokehConstants {
	float range;
};

struct XFloatCompressConstants {
	int count;
	uint32_t SourceBinding;
	uint32_t Destination;
	int avgFlags;
};

struct MaxFilterConstants {
	int kernelSize;
	int alphaOnly;
};

struct UpdateChunkMappingsConstants {
	glm::vec3 worldPosition;
	int quality;
	glm::vec3 lowPosition;
	int chunkIndex;
	glm::vec3 highPosition;
	int forw;
	int back;
	int left;
	int right;
	int up;
	int down;
};
struct UpdateChunkMappingsBulkConstants {
	glm::vec3 position;
	int FILLER1;
	glm::vec3 lowPosition;
	int FILLER2;
	glm::vec3 highPosition;
	int FILLER3;
	int quality;
	int chunkIndex;
	int forw;
	int back;
	int left;
	int right;
	int up;
	int down;
};

// This is the internal GPU model of chunk info
struct UpdateChunkMappingsStruct {
	glm::vec3 world_image_position;
	int quality;
	glm::vec3 worldPosition;
	int forw;
	glm::vec3 highPosition;
	int back;
	int left;
	int right;
	int up;
	int down;
};


//struct DownscaleOctreeConstants {
//	glm::ivec3 gran;
//	int PACKING2;
//	glm::ivec3 chunkDimensions;
//	int PACKING3;
//	int ChunkID;
//	int mipToStartOctree;
//	int mipToEndOctree;
//	int BlockBitHighest; //all block bits set to 1
//	int BlocksBits;      //number of block bits
//	int OctreeBitHighest;//all octree bits set to 1
//
//	int DetailMips;
//	float DimensionScale;
//};

struct VacantChunkGeneratorConstants {
	glm::ivec3 gran;
	int maxMip;
	glm::ivec3 lowPosition;
	int DetailMips;
	int BlocksBits;
};

struct InitialScanConstants {
	glm::ivec3 chunkDimensions;
	int NetworkingStart;
	glm::vec3 extent;
	int WorldMipTotal;
	glm::ivec2 ImageSize;
	int DetailMipTotal; // is this not the same as boundry between world?
	int MipsUnderOne; //total number of mips with resolution < 1 is a negative number
	int Scale;
	float renderScale;
	int BlockBits; //Block bits all 1
	int BlockBitHighest; //Block bits all 1
	int OctreeBitHighest;//Octree bits all 1
	int mipBoundryBetweenWorldAndDetail;

	int max_look_increase_quality;
	int List_Max_Length;
	int List_Start;
};

struct InitialScanData {
	glm::vec3 position;
	int quality;
	glm::vec3 normal;
	float dist;
	int networkID;
	int voxelID;
	int PADDING2;
	int PADDING3;
};	

struct ChunkHeader {
	glm::ivec3 position;
	UINT32 ptr; // first bit ptr or constant value. 30 bit pointer to data.
};

struct HeaderUpdate {
	glm::vec3 pos{};
	int lowestIndex{};
	int NumberPerAxii;
};

struct CreateChunkConst {
	glm::ivec3 position;
	int ptrInMem;
	VkAccelerationStructureInstanceKHR instance;
};