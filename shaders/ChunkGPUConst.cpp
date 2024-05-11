#include "ChunkGPUConst.h"

float QualityToSize(int quality) {
    quality += MipsUnderOne;
    return pow(4, quality) * 2;
}

int ChunkHeaderIndex(ivec3 pos)
{
	int index = pos.x;
	index += pos.y << BitsPerAxii;
	index += pos.z << BitsPerAxii;
	return index;
}

int GetChunkVolumeIndexFromPos(ivec3 volume_pos, ivec3 pos) {
	pos -= volume_pos;

	return ChunkHeaderIndex(pos);
}
