#include "ChunkGPUConst.h"

float QualityToSize(int quality) {
    quality += MipsUnderOne;
    return pow(4, quality) * 2;
}

float SizeToQuality(float size)
{
	return  ceil(log2(size / 2.0) / 2.0); // Log2(A/2)/Log2(4)
}

int ChunkHeaderIndex(ivec3 pos)
{
	int index = pos.x;
	index += pos.y << BitsPerAxii;
	index += pos.z << (BitsPerAxii * 2);
	return index;
}

int GetChunkVolumeIndexFromPos(ivec3 volume_pos, ivec3 pos) {
	pos -= volume_pos;

	return ChunkHeaderIndex(pos);
}
