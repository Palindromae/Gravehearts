//#include "../ChunkGPUConst.h"
//#include "../VoxelBrick.h"


//2097152; // 2^31 / 1024
uint getChapterID(uint hash){
	return hash&uint(chapters-1);
	//return int(mod(int(hash*k_prime),int(chapters)));
	//(x*y)>>(MAXIMAL BIT IN X) where 2
}

int positionalHash(vec3 chunk_pos)
{
	return int(pow(chunk_pos.x*2437,2)) + int(pow(chunk_pos.y*1907,4)) + int(pow(chunk_pos.z*3169, 6));
}

uint CombinePositionalAndIDHash(uint ID, uint positional_hash){

	return ID + positional_hash;
}

//ivec3 IndexToVoxelBrickCoords(int ptr)
//{
//	ivec3 coord;/

//	coord.x = ptr & VBC_mask;
//	coord.y = (ptr>>10) & VBC_mask;
//	coord.z = (ptr>>20) & VBC_mask;
	
//	return coord * brick_size;
//}

/*
int VoxelBrickCoordsToIndex(ivec3 coord)
{
	coord = ivec3(coord*brick_size_inv);

	int ptr;

	ptr = coord.x;
	ptr += (coord.y)<<10;
	ptr += (coord.z)<<20;
	
	return ptr;
}
*/

vec3 decode_pos(int encoded_pos){
    
    vec3 pos = vec3(0);

    pos.x = encoded_pos & 255;
    pos.y = (encoded_pos >> 8)  & 255;
    pos.z = (encoded_pos >> 16) & 255;

    return pos * MinSize; 
}

int encode_pos(vec3 pos){
    
    ivec3 p = ivec3(pos*greatestZoom);

    int p_e = 0;
    p_e =   p.x  & 255;
    p_e += (p.y & 255) << 8;
    p_e += (p.z & 255) << 16;

    return p_e;
}

vec3 decode_local_ID(int encoded_pos, float scale){
	vec3 offset = vec3(0);

    offset.x =  encoded_pos & 3;
    offset.y = (encoded_pos >> 2) & 3;
    offset.z = (encoded_pos >> 4) & 3;

	return offset * scale;
}