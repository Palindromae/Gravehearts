#pragma once
#include "ComputeShader.h"

namespace Shaders
{
	enum ShaderClass {
		CopyWorldToBuffer,
		Compact1024,
		Scan1024,
		VoxelCompression,
		AttachKeysToCompression,
		WriteUpdatesToBuffer,
		AddTexture,
		Threshold,
		AverageImage, 
		MultiplyByConstant,
		XFloatCompress,

		// Blurs
		CircularBokeh,
		Downsample13,
		Upsample3x3,
		MaxFilter,

		// PostProcessing
		BloomReconstitute,
		WhiteBalance,
		Vignette,
		ChromaticAberration,
		DofStart,
		DepthOfField,
		Exposure,

		// Rendering

		InitialScan,

		// Tone-Mapping
		Reinhard,
		ReinhardExtended,
		TumblinRushmeier,
		Schlick,
		Ward,
		HableFilmic,
		HableUpdated,
		UchimuraFilmic,
		HillAces,
		NarkowiczACES,
		JodieReinhard,
		AcesFilmic,

		// Chunks
		WriteNewChunkVolume,
		UpdateChunkVolume,

		// SVO
		VoxelBrickMemoryAllocator,
		OverworldBrickSetter,
		CopyListToVoxelBrickInDirect,

		// Terrain Generation
		GenerateHeightMap,
		SetOverworld3DNoise,
		OverworldBlockSetter,
		VoxelOctreeAllForOne,
		VoxelOctreeAllForOneWithSafetyMips,

		// Utility
		ReadTexture,
		ReadChunk,
		ReadChunkInternal,
		CopyListToIndirect,
		ZeroOutBuffer,
		ZeroOutImage,


		END
	};
	const extern ComputeShader** Shaders;
}