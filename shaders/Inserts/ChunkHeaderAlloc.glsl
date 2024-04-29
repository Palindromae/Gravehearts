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

void MutexChunkHeaders(){

	int obtained = 1;
		do {
			obtained = atomicCompSwap(ChunkHeaders[0].position.x, 0 , 1);
		}
	while (obtained == 1);
}

void ReleaseChunkHeaders(){
	ChunkHeaders[0].position.x = 0;
}

int attemptToObtainChunkHeader(vec3 pos){

	int chunk_header_id = -1;

	if(ChunkHeaders[0].position.y >= NoChunks)
		return chunk_header_id;


	MutexChunkHeaders();

	//SECURED
	if(ChunkHeaders[0].position.y < NoChunks)
	{
		// space for one more chunk
		ChunkHeaders[0].position.y++;

		chunk_header_id = 1 + ChunkHeaders[0].ptr;

		int next_header = ChunkHeaders[chunk_header_id].ptr;

		if(next_header == 0){
			next_header = chunk_header_id + 1;
		}

		ChunkHeaders[0].ptr = next_header;

	}

	ReleaseChunkHeaders();
	
	// RELEASED

	ChunkHeaders[chunk_header_id].position = pos;	

	return chunk_header_id;
}


