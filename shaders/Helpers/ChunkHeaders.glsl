
//struct ChunkHeader{
//	vec3 position;
//	uint ptr; // first bit ptr or constant value. 30 bit pointer to data.
//};
const uint MAXUINT = 0xffffffff;
struct ChunkHeader{
	vec3 position;
	uint ptrs[125];
};

const int NoChunks = 3125000; //50mb/16bytes 

