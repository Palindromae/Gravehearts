
//#include "../ChunkGPUConst.h"


//struct ChunkHeader{
//	vec3 position;
//	uint ptr; // first bit ptr or constant value. 30 bit pointer to data.
//};

const int NoChunksPtrs = 512;

const uint MAXUINT = 0xffffffff;
struct ChunkHeader{
	ivec3 position;
	uint ptrs[NoChunksPtrs];
};

const int NoChunks = 3125000; //50mb/16bytes 

ivec3 HeaderPtrIDToPosition(uint id){
	
	ivec3 pos;

	pos.x = int(id & 255);
	pos.y = int (pos.y >> BitsPerAxii) & 255;
	pos.z = int (pos.z >> BitsPerAxii) & 255;

	return pos;
}