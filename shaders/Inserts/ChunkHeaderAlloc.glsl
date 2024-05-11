// #include Helpers/ChunkHeader.glsl
// ChunkHeaders (ChunkHeader)

//struct ChunkHeader{
//	vec3 position; 
//	uint ptr;
//};

// Header
// position.x -> mutex
// position.y -> counter


// For now this isnt used instead, a CPU driven version will take its place.

void ClearHeader(int volume, int chunkID){
	ChunkHeaders[volume].ptrs[chunkID] = MAXUINT;

}