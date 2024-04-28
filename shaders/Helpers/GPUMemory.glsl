const int chapters = 512;
const int TotalMemory = 512000000; //bytes 
const int chapter_size = TotalMemory / chapters;
const int page_size = 16; // 8 nodes two ints
const int header_size = 2; 
const uint k_prime = 7919u;

const int ptr_mask = 1073741823;

const int leaf_value = 1<<30;
const int const_value = 1<<31;
const int const_leaf_value = 3<<30;

const int brick_size = 8; // make sure to change brick setters work groups
const float brick_size_inv = 1/brick_size; 
const int brick_size_mask = brick_size-1; 
const int brick_size_bits = 3; 

const int VBC_side_length = 1024;
const int VBC_mask = (1<<10)-1;

struct UnexpandedNode{
    int position; // 8 bits for x then y then z
    int mempos;
};


//2097152; // 2^31 / 1024
int getChapterID(uint hash){
	return int(mod(int(hash*k_prime),int(chapters)));
}

int positionalHash(vec3 chunk_pos)
{
	return int(pow(chunk_pos.x*2437,2)) + int(pow(chunk_pos.y*1907,4)) + int(pow(chunk_pos.z*3169, 6));
}

int CombinePositionalAndIDHash(int ID, int positional_hash){

	return ID + positional_hash;
}

ivec3 IndexToVoxelBrickCoords(int ptr)
{
	ivec3 coord;

	coord.x = ptr & VBC_mask;
	coord.y = (ptr>>10) & VBC_mask;
	coord.z = (ptr>>20) & VBC_mask;
	
	return coord * brick_size;
}

int VoxelBrickCoordsToIndex(ivec3 coord)
{
	coord = ivec3(coord*brick_size_inv);

	int ptr;

	ptr = coord.x;
	ptr += (coord.y)<<10;
	ptr += (coord.z)<<20;
	
	return ptr;
}

vec3 decode_pos(int encoded_pos){
    
    vec3 pos = vec3(0);

    pos.x = encoded_pos & 255;
    pos.y = (encoded_pos >> 8)  & 255;
    pos.z = (encoded_pos >> 16) & 255;

    return pos * 0.03125; // 1/32
}

int encode_pos(vec3 pos){
    
    ivec3 p = ivec3(pos*32);

    int p_e = 0;
    p_e = p.x  & 255;
    p_e += (p.y & 255) << 8;
    p_e += (p.z & 255) << 16;

    return p_e;
}
