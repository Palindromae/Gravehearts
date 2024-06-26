#include "stdafx.h"
#include "ChunkConst.h"
#include <vulkan/vulkan_core.h>
#include "DistanceCalculation.h"

float nve::Chunks::calculateResolution(int mip) {
	return  pow(2, mip + greatestResolutionBitOffset);
}

float nve::Chunks::calculateZoom(int mip) {
	return  pow(.5, mip + greatestResolutionBitOffset);
}

void nve::Chunks::CreateDistanceToQualityArray()
{
	int max_distance = maxRenderDistanceFromMip(maxNumberOfWorldMips)+1;
	DistanceToQuality = (int*)malloc(sizeof(int) * max_distance);

	int pre = 0;
	for (size_t i = 0; i < maxNumberOfWorldMips; i++)
	{
		int max = maxRenderDistanceFromMip(i);
		for (size_t d = pre; d <= max; d++)
		{
			DistanceToQuality[d] = i;

		}
		pre = max+1;
	}
}

float nve::Chunks::calculateGreatestResolutionFromChunkDistance(float distance) {
	static const double l2 = log(2);
	//this could be a bit slow but for now it should do
	return std::max(static_cast<double>(greatestResolution), pow(2, floor(log(distance / RenderDistanceScale) / l2)));
}
int nve::Chunks::maxRenderDistanceFromMipWithNonConstantSize(int mip, int bitOffset) {
	return ceil(RenderDistanceScale * pow(2, mip + bitOffset));
}

int nve::Chunks::maxRenderDistanceFromSizeWithNonConstantSize(int size, int bitOffset) {
	return ceil(RenderDistanceScale * size);
}
int nve::Chunks::maxRenderDistanceFromMip(int mip) {
	return ceil(RenderDistanceScale * pow(2, mip + greatestResolutionBitOffset));
}

int nve::Chunks::maxRenderDistanceFromSize(float size) {
	return ceil(RenderDistanceScale * size);
}
//https://www.desmos.com/calculator/arlbur8g2g
int nve::Chunks::calculateLowestQualityFromChunkDistance(float distance)
{
	int max_distance = maxRenderDistanceFromMip(maxNumberOfWorldMips);

	if (distance >= max_distance)
		return maxNumberOfWorldMips;

	return DistanceToQuality[(int)distance];

	distance = std::max(greatestResolution,std::max(0.0f, distance)/RenderDistanceScale);
	int a = static_cast<int>(ceil(log2(distance)));
	float b = log2(distance);
	return a;
}

int nve::Chunks::calculateLowestQualityForChunk(const glm::vec3& chunk_coord_in_chunk_space, const glm::vec3& player_position_chunk_pos)
{
	glm::vec3 vector = (chunk_coord_in_chunk_space - player_position_chunk_pos) * chunk_dimensions;
	
	int dist = CalculateChunkDistance(vector);
	
	return calculateLowestQualityFromChunkDistance(dist);
}


int nve::Chunks::calculateLowestMipFromChunkDistance0Unsafe(float distance)
{

	int max_distance = maxRenderDistanceFromMip(maxNumberOfWorldMips);

	if (distance >= max_distance)
		return maxNumberOfWorldMips;

	return DistanceToQuality[(int)distance];

	distance = std::max(0.0f, distance) / RenderDistanceScale;
	return static_cast<int>(ceil(log2(distance)));
}

int nve::Chunks::calculateWorldDiameter(int radii, bool centered) {
	return 2 * radii + centered;
}

float nve::Chunks::calculateGreatestResolutionFromChunkDistancePortal(float distance) {
	static const double l2 = log(2);
	//this could be a bit slow but for now it should do
	return std::max(static_cast<double>(PortalLowestResolution), pow(2, floor(log(distance / RenderDistanceScale) / l2)));
}

int nve::Chunks::calculateLowestMipFromChunkDistancePortal(float distance) {
	static const double l2 = log(2);
	//this could be a bit slow but for now it should do
	return std::max(0, static_cast<int>(floor(log(distance / RenderDistanceScale) / l2)) - greatestResolutionBitOffset);
}

glm::ivec3 nve::Chunks::calculateMaximumRenderDistance(bool center) {
	int a = static_cast<int>(maxRenderDistanceFromSize(maxMipSize));

	return { a,(a - 1) / 2 + 1,a };
}
glm::vec3 nve::Chunks::WorldSpaceToChunkSpace(glm::vec3 worldSpace)
{
	return glm::floor(worldSpace / chunk_dimensions);
}
glm::vec3 nve::Chunks::CalculateBulkSize(int mip, int NoiseScaleMultiplierMip)
{
	return glm::vec3(glm::pow(2, std::max(0, mip) + MipsUnderOne));
}
int nve::Chunks::CalculateChunkDistance(glm::vec3 vector)
{
	vector.y *= chunk_scaler.y; //This is to scale the chunk as chunks are twice as tall as they are wide
	return ChebyshevDistance(vector);
}
float nve::Chunks::CalculateEuclideanChunkDistance(glm::vec3 vector)
{
	vector.y *= chunk_scaler.y; //This is to scale the chunk as chunks are twice as tall as they are wide
	return EuclideanDistance(vector);
}
glm::uvec3 nve::Chunks::alignedDivision(const VkExtent3D& extent, const VkExtent3D& granularity)
{
	glm::uvec3 res;
	res.x = extent.width / granularity.width + ((extent.width % granularity.width) ? 1u : 0u);
	res.y = extent.height / granularity.height + ((extent.height % granularity.height) ? 1u : 0u);
	res.z = extent.depth / granularity.depth + ((extent.depth % granularity.depth) ? 1u : 0u);
	return res;
}

bool nve::Chunks::IsWorldMip(int mip)
{
	return mip >= WorldSizeResolutionBitOffset;
}

bool nve::Chunks::IsHighQuality(int quality) {
	return quality + MipsUnderOne < mipBoundryBetweenWorldAndDetail;
}