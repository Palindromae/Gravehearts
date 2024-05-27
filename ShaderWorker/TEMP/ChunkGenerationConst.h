#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
#include "ChunkGPUConst.h"
// GLSL Type
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif








////////////////// HEIGHTMAP CONSTANTS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HeightmapOctaves {

	vec2 offset;
	float frequency;
	float amplitude;

};


const int heightmap_noiseSize = 2;
const float SEA_LEVEL = 0;


const int heightmap_octaves_no = 3;
const HeightmapOctaves heightmap_octaves[heightmap_octaves_no] = {
	{vec2(5000, 300), 1 / 16.0, 12.0},
	{vec2(-556, 500), 1 / 8, 4},
	{vec2(400, -666), 1/2, 1} 
};
//Padding for each side of an axis, total padding is x2
const int heightmap_dimension_padding = 1;
const ivec2 heightmap_dimensions = ivec2(chunk_dimensions_int.x,chunk_dimensions_int.z) / heightmap_noiseSize + ivec2(heightmap_dimension_padding)*2;


////////////////// 3D NOISE CONSTANTS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Noise3DOctaves {

	vec3 offset;
	float frequency;
	float contribution;
};

const int noise3d_noiseSize = 2;
const float isolevel = .5;
const int noise3d_dimension_padding = 1;
const ivec3 noise3d_dimensions = chunk_dimensions_int / noise3d_noiseSize + ivec3(heightmap_dimension_padding) * 2;

const int noise3d_octaves_no = 2;
const Noise3DOctaves noise3d_octaves[noise3d_octaves_no] = {
	{vec3(666,-999, -666), 1 / 17.1771, 3},
	{vec3(234,-9000, 5555), 1 / 3.333, 1}
};