// #include "Helpers/GPUMemory.glsl"
// GPUMemory (VoxelBrick)

// WARNING THIS CANNOT BE RAN WITHIN A LOCAL THREAD!
// IF ONE LOCAL THREAD WAITS ON ANOUTHER IT DEADLOCKS

void MutexChapter(uint chapterIndex){

	uint obtained = 1;
		do {
			obtained = atomicCompSwap(GPUMemory[chapterIndex].ptrs[0],0,1);
		}
	while (obtained == 1);
}

void ReleaseChapter(uint chapterIndex){
	GPUMemory[chapterIndex].ptrs[0] = 0;
}

// Memory condition isnt guaranteed for anything except for the mask
uint attemptToObtainChapterPage(uint chapter){

	uint chapterIndex = chapter * chapter_size;

	bool allowed = false;
	int counted = 0;

	do {
		// Try to find a chapter with memory avalible
		do {
		
			if(GPUMemory[chapterIndex].ptrs[0] != 2) // Found a valid chapter
				break;

			counted++;
			chapterIndex += chapter_size;

		} while (counted < chapters);

		// Failed to obtain a chapter, all full. THIS IS A BAD STATE
		if (counted == chapters)
			return -1;


		// Wait until chapter is secured
		MutexChapter(chapterIndex);

	 // Validate the chapter has space left
	 if(GPUMemory[chapterIndex].ptrs[0] == 2)
	 {
		ReleaseChapter(chapterIndex);
		continue;
	 }

	 // Chapter is secured and valid so exit out of loop
	 break;

	} while (counted < chapters); 


	 if (counted == chapters) // failed to obtain chapter BAD STATE complete OOM
		return -1;

	 // SECURED DO NOT RETURN WITHOUT UNFLAGGING

	 uint offset = GPUMemory[chapterIndex].ptrs[1] + header_size;


	 // Memory obtained, now merge lists to head
	 uint secured_chapter = chapterIndex + offset;

	 uint nextOffset = GPUMemory[secured_chapter].ptrs[1]; // offset > 1 

	 if(nextOffset == 0){
		nextOffset = offset;
	 }


	 // Out of Memory ||  OOM
	 if(nextOffset >= chapter_size){
		GPUMemory[chapterIndex].ptrs[0] = 2;
		GPUMemory[chapterIndex].ptrs[1] = 0;
	 } else {
	    // SET NEXT MEMORY
		GPUMemory[chapterIndex].ptrs[1] = nextOffset;
		ReleaseChapter(chapterIndex);
	 }
	 // UNSECURED FINE TO RETURN


	 //GPUMemory[secured_chapter].ptrs[0] = 0; Probably not required? Mask should prevent reads
	// GPUMemory[secured_chapter].ptrs[1] = 0;
	 GPUMemory[secured_chapter].mask    = 0;

	 return secured_chapter;
}

void FreeChapterPage(uint page_index)
{
	uint chapterIndex = page_index / chapter_size;


	// Secured Return
	MutexChapter(chapterIndex);
     
	// Alter the chapters to be added to linked list

	// Bind the current list to the page
	//GPUMemory[page_index].ptrs[0] = 0; // Shouldnt be required
	GPUMemory[page_index].ptrs[1] = GPUMemory[chapterIndex].ptrs[1]; // set offset to the previous next page.

	//Bind the page to the header
	GPUMemory[chapterIndex].ptrs[1] = page_index - 1;

	//Unsecured
	ReleaseChapter(chapterIndex);
}

void CleanNodeAndReturn(uint page_index, uint parent_node, uint indexInParent){
	
	//GPUMemory[parent_node].ptrs[indexInParent]  = 0; Might not be needed, the mask should prevent it being read
	GPUMemory[parent_node].mask &= ~(1<<indexInParent); // Create a bit mask of all ones excet the target e.g. 1110111

	FreeChapterPage(page_index);
}