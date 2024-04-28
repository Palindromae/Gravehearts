// #include "Helpers/GPUMemory.glsl"
// GPUMemory (ints)


void MutexChapter(int chapterIndex){

	int obtained = 1;
		do {
			obtained = atomicCompSwap(GPUMemory[chapterIndex],0,1);
		}
	while (obtained == 1);
}

void ReleaseChapter(int chapterIndex){
	GPUMemory[chapterIndex] = 0;
}

int attemptToObtainChapterPage(int chapter){

	int chapterIndex = chapter * chapter_size;

	bool allowed = false;
	int counted = 0;

	do {
		// Try to find a chapter with memory avalible
		do {
		
			if(GPUMemory[chapterIndex] != 2) // Found a valid chapter
				break;

			counted++;
			chapterIndex += chapter_size;

		} while (counted < chapters);

		// Failed to obtain a chapter, all full. THIS IS A BAD STATE
		if (counted == chapters)
			return -1;


		// Wait until chapter is secured
		MutexChapter(chapterIndex);

	 if(GPUMemory[chapterIndex] != 2)
	 {
		ReleaseChapter(chapterIndex);
		break;
	 }


	} while (counted < chapters); 


	 if (counted == chapters) // failed to obtain chapter BAD STATE complete OOM
		return -1;

	 // SECURED DO NOT RETURN WITHOUT UNFLAGGING

	 int offset = GPUMemory[chapterIndex + 1] + header_size;


	 // Memory obtained, now merge lists to head
	 int secured_chapter = chapterIndex + offset;

	 int nextOffset = GPUMemory[secured_chapter]; // offset > 1 Offset has built in + 2 to account of header

	 if(nextOffset == 0){
		nextOffset = offset + page_size;
	 }


	 // Out of Memory ||  OOM
	 if(nextOffset >= chapter_size){
		GPUMemory[chapterIndex]     = 2;
		GPUMemory[chapterIndex + 1] = 0;
	 } else {
	    // SET NEXT MEMORY
		GPUMemory[chapterIndex + 1] = nextOffset;
		GPUMemory[chapterIndex]     = 0;
	 }
	 // UNSECURED FINE TO RETURN


	 GPUMemory[secured_chapter]   = 0;
	 GPUMemory[secured_chapter+1] = 0;

	 return secured_chapter;
}