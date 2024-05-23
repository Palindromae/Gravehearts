#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif


const int NoChunksPerAxii = 8; //set ChunkVolumeGPU and chunk Header before changing and no masks and No Chunks Volumes Total
const int BitsPerAxii = 3;
const int NoChunksPerVolume = NoChunksPerAxii * NoChunksPerAxii * NoChunksPerAxii;



// WARNING DO NOT ADJUST ANY OF THESE NUMBERS WITHOUT A RECALCULATION OF ALL OF THEM
const vec3 chunk_dimensions = vec3(32,32,32);
const ivec3 chunk_dimensions_int = ivec3(32,32,32);
const float greatestZoom = 8.0f;
const float minimumResolution = 32;
const float MinSize = .125;
const int MaxQuality = 4; // lowest quality (0) is the id of the node that is 1m size with 1/8th size voxels in its bricks This isnt a const, just hard set off of ChunkSize and NoChildrenPerBrick
const int MipsUnderOne = -2 ;

// Define functions

#ifdef __cplusplus
float QualityToSize(int quality);
// 
int GetChunkVolumeIndexFromPos(ivec3 volume_pos, ivec3 pos);
int ChunkHeaderIndex(ivec3 pos);
#else 
float QualityToSize(float quality) {
    quality += MipsUnderOne;
    return pow(4.0, quality) * 2;
}

float QualityToSizePtr(float quality) {
	return pow(4.0, quality) * 2;
}

int ChunkHeaderIndex(ivec3 pos) {
	int index = pos.x;
	index += pos.y << BitsPerAxii;
	index += pos.z << (BitsPerAxii * 2);

	return index;
}

ivec3 GetHeaderOffset(uint index) {
	ivec3 position;

	int idx = int(index);

	position.x = idx & (NoChunksPerAxii - 1);
	position.y = (idx >>BitsPerAxii) & (NoChunksPerAxii - 1);
	position.z = (idx >> BitsPerAxii * 2) & (NoChunksPerAxii - 1);

	return position;
}

int GetChunkVolumeIndexFromPos(ivec3 volume_pos, ivec3 pos) {
	pos -= volume_pos;
	return ChunkHeaderIndex(pos);
}


int GetVoxelBrickIndex(vec3 position, vec3 parent_position, float scale) {
	position /= scale;
	ivec3 n_pos = ivec3(floor(position));

	int index = n_pos.x;
	index += n_pos.y << BitsPerAxii;
	index += n_pos.z << (BitsPerAxii*2);

	return index;
}

#endif


